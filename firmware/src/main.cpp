
#include <Arduino.h>
#include <OneButton.h>
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

OneButton resetButton(globalConfig.resetPin, true);

void onResetPressStart()
{
  Serial.println("[RESET] Hold Start : " + String(millis()));
  globalState.lastResetButtonTime = millis();
}

void onResetPressStop()
{
  Serial.println("[RESET] Hold Stop : " + String(millis()));
  unsigned long diff = millis() - globalState.lastResetButtonTime;
  int holdTimeSeconds = diff / 1000.0f;
  Serial.println("[RESET] Hold Time : " + String(holdTimeSeconds) + "s");
  if (holdTimeSeconds >= 5)
  {
    clearConfig();
    ESP.restart();
  }
}

void onResetDoubleClicked()
{
  Serial.println("[RESET] Double Clicked : " + String(millis()));
  if (!globalState.bleRunning)
  {
    if (!globalState.bleSetup)
    {
      Serial.println("[RESET] Setup Provisioning");
      setupProvisioning();
    }
    Serial.println("[RESET] Start Provisioning");
    startProvisioningServer();
  }
  else
  {
    Serial.println("[RESET] Stop Provisioning");
    stopProvisioningServer();
    //TODO: Find a way to stop advertising
    Serial.println("[RESET] Restarting");
    ESP.restart();
  }
}

void setup()
{
  // Setup reset pin
  Serial.begin(115200);

  Serial.print("[INIT] App Version: ");
  Serial.print(VERSION);
  Serial.print(" - ");
  Serial.println(VERSION_NAME);

  setupConfig();
  resetButton.attachLongPressStart(onResetPressStart);
  resetButton.attachLongPressStop(onResetPressStop);
  resetButton.attachDoubleClick(onResetDoubleClicked);

  globalState.lastResetButtonTime = millis();

  pinMode(globalConfig.lightPin, OUTPUT);
  ledcSetup(globalConfig.lightLedcChannel, globalConfig.lightLedcFrequency, 8);
  ledcAttachPin(globalConfig.lightPin, globalConfig.lightLedcChannel);

  Serial.print("[INIT] Device ID: ");
  Serial.println(globalConfig.deviceName);

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

    if (globalState.bleRunning)
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
  resetButton.tick();
}
