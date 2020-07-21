#include "IoTCore.h"

#include <Client.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>

#include "DeviceConfig.h"
#include "DeviceState.h"

Client *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iat = 0;
String jwt;

void messageReceived(String &topic, String &payload)
{
  globalState.connected = true;
  Serial.println("[MQTT] incoming: " + topic + " - " + payload);

  if (payload.length() == 0)
  {
    Serial.println("[MQTT] Empty payload");
    return;
  }

  if (!topic.endsWith("/commands"))
  {
    Serial.println("[MQTT] Handling only commands topic");
    return;
  }

  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.println("[MQTT] Failed to parse json");
    return;
  }
  if (doc.containsKey("power"))
  {
    bool on = doc["power"] | false;
    updateLampState(on);
  }
  if (doc.containsKey("brightness"))
  {
    int lampBrightness = doc["brightness"] | 0;
    updateLampBrightness(lampBrightness);
  }
}

String getJwt()
{
  iat = time(nullptr);
  Serial.println("[MQTT] Refreshing JWT");
  jwt = device->createJWT(iat, globalConfig.jwtExpSecs);
  return jwt;
}

void publishState(String data)
{
  if (mqtt != NULL && mqttClient != NULL)
  {
    if (mqttClient->connected())
    {
      Serial.println("[MQTT] Publishing data to IoT Core");
      mqtt->publishTelemetry(data);
    }
  }
}

void mqttTask(void *p)
{
  while (1)
  {
    if (mqtt != NULL && mqttClient != NULL)
    {
      mqtt->loop();
      vTaskDelay(10 / portTICK_PERIOD_MS);

      if (globalState.online && !mqttClient->connected())
      {
        Serial.println("[MQTT] Not Connected, trying to connect to IoT Core...");
        globalState.connected = false;
        mqtt->mqttConnectAsync();
      }
      bool connected = mqttClient->connected();
      if (globalState.connected != connected)
      {
        String state = connected ? "connected" : "not connected";
        Serial.println("[MQTT] Connection state changed : " + state);
        globalState.connected = connected;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setupCloudIoT()
{
  if (strlen(globalConfig.iotCorePrivateKey) == 0)
  {
    Serial.println("[MQTT] Missing IoT Core Private Key");
    return;
  }

  if (strlen(globalConfig.iotCoreProjectId) == 0)
  {
    Serial.println("[MQTT] Missing IoT Core Project Id");
    return;
  }

  if (strlen(globalConfig.iotCoreRegion) == 0)
  {
    Serial.println("[MQTT] Missing IoT Core Region");
    return;
  }

  if (strlen(globalConfig.iotCoreRegistry) == 0)
  {
    Serial.println("[MQTT] Missing IoT Core Registry Id");
    return;
  }

  if (!globalState.online)
  {
    Serial.println("[MQTT] Not online");
    return;
  }

  device = new CloudIoTCoreDevice(globalConfig.iotCoreProjectId, globalConfig.iotCoreRegion, globalConfig.iotCoreRegistry, globalConfig.deviceName, globalConfig.iotCorePrivateKey);

  netClient = new WiFiClientSecure();
  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
  mqtt->setUseLts(true);
  mqtt->startMQTT();

  globalState.connected = false;

  Serial.println("[MQTT] Client started");
}
