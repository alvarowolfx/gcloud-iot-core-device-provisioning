
#include <Arduino.h>
#include <ArduinoJson.h>
#include <rom/rtc.h>

#include "Network.h"
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

  Serial.print("App Version: ");
  Serial.print(VERSION);
  Serial.print(" - ");
  Serial.println(VERSION_NAME);

  setupConfig();
  pinMode(globalConfig.resetPin, INPUT_PULLUP);
  pinMode(globalConfig.lightPin, OUTPUT);
  ledcSetup(globalConfig.lightLedcChannel, globalConfig.lightLedcFrequency, 8);
  ledcAttachPin(globalConfig.lightPin, globalConfig.lightLedcChannel);

  Serial.print("Device ID: ");
  Serial.println(globalConfig.deviceName);

  if (digitalRead(globalConfig.resetPin) == LOW)
  {
    Serial.println("Clear Settings...");
    clearConfig();
    delay(2000);
  }

  lastResetReason = rtc_get_reset_reason(0);
  if (lastResetReason == POWERON_RESET)
  {
    Serial.println("Setup Provisioning");
    setupProvisioning();
    Serial.println("Start Provisioning");
    startProvisioningServer();
  }
  loadConfig();
}

void loop()
{
  bool error = reconnectWiFi();
  bool online = !error;
  if (globalState.online != online)
  {
    globalState.online = online;
    if (error)
    {
      Serial.println("Error connecting to WiFi");
      delay(1000);
    }
    else
    {
      Serial.println("Connected to WiFi");
      delay(100);
    }
  }

  if (globalState.hasChanges)
  {
    globalState.hasChanges = false;
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
      StaticJsonDocument<512> doc;
      doc["id"] = globalConfig.deviceId;
      doc["wifiSsid"] = globalConfig.wifiSsid;
      doc["online"] = globalState.online;
      doc["connected"] = globalState.connected;
      doc["power"] = globalState.lampState;
      doc["brightness"] = globalState.lampBrightness;
      String output;
      serializeJson(doc, output);
      updateBleStatus(output);
    }
  }
}
