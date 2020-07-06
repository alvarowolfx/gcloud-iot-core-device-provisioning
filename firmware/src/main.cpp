
#include <Arduino.h>
#include <ArduinoJson.h>
#include <rom/rtc.h>

#include "Network.h"
#include "NTP.h"
#include "IoTCore.h"
#include "Provisioning.h"
#include "DeviceConfig.h"
#include "DeviceState.h"
#include "OTAUpdate.h"

#define POWERON_RESET 1

RESET_REASON lastResetReason;
DeviceConfig globalConfig;
DeviceState globalState;

void setup()
{
  // Setup reset pin
  Serial.begin(115200);

  Serial.print("[INIT] App Version: ");
  Serial.print(VERSION);
  Serial.print(" - ");
  Serial.println(VERSION_NAME);

  setupConfig();
  pinMode(globalConfig.resetPin, INPUT_PULLUP);
  pinMode(globalConfig.lightPin, OUTPUT);
  ledcSetup(globalConfig.lightLedcChannel, globalConfig.lightLedcFrequency, 8);
  ledcAttachPin(globalConfig.lightPin, globalConfig.lightLedcChannel);

  Serial.print("[INIT] Device ID: ");
  Serial.println(globalConfig.deviceName);

  if (digitalRead(globalConfig.resetPin) == LOW)
  {
    Serial.println("[INIT] Clear Settings...");
    clearConfig();
    delay(2000);
  }

  lastResetReason = rtc_get_reset_reason(0);
  if (lastResetReason == POWERON_RESET)
  {
    Serial.println("[INIT] Setup Provisioning");
    setupProvisioning();
    Serial.println("[INIT] Start Provisioning");
    startProvisioningServer();
  }
  loadConfig();

  xTaskCreate(networkTask, "network", 4096, NULL, 5, NULL);
  xTaskCreate(mqttTask, "mqtt", 4096 * 4, NULL, 5, NULL);
  xTaskCreate(timeTask, "time", 4096, NULL, 4, NULL);
}

void loop()
{
  if (globalState.hasStateChanges)
  {
    globalState.hasStateChanges = false;
    if (globalState.lampState)
    {
      int dutyCycle = map(globalState.lampBrightness, 0, 100, 0, 255);
      ledcWrite(globalConfig.lightLedcChannel, dutyCycle);
    }
    else
    {
      ledcWrite(globalConfig.lightLedcChannel, 0);
    }

    if (globalState.bleReady)
    {
      StaticJsonDocument<256> doc;
      doc["id"] = globalConfig.deviceId;
      doc["wifiSsid"] = globalConfig.wifiSsid;
      doc["online"] = globalState.online;
      doc["connected"] = globalState.connected;
      doc["power"] = globalState.lampState == HIGH;
      doc["brightness"] = globalState.lampBrightness;
      String output;
      serializeJson(doc, output);
      updateBleStatus(output);
    }

    if (globalState.connected)
    {
      StaticJsonDocument<128> doc;
      doc["power"] = globalState.lampState == HIGH;
      doc["brightness"] = globalState.lampBrightness;
      String output;
      serializeJson(doc, output);
      publishState(output);
    }
  }
}
