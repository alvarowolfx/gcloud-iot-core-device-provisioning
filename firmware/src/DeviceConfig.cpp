#include "DeviceConfig.h"

#include <ArduinoJson.h>

const char *configFilePath = "/config.json";

bool saveConfig()
{
  File file = SPIFFS.open(configFilePath, FILE_WRITE);
  if (!file)
  {
    Serial.println("[CONFIG] Failed to create file to be saved");
    return true;
  }

  StaticJsonDocument<512> doc;

  doc["wifiSsid"] = globalConfig.wifiSsid;
  doc["wifiPass"] = globalConfig.wifiPass;
  doc["iotCorePrivateKey"] = globalConfig.iotCorePrivateKey;

  if (serializeJson(doc, file) == 0)
  {
    Serial.println("[CONFIG] Failed to save config to file");
  }

  file.close();
  return false;
}

bool clearConfig()
{
  globalConfig = DeviceConfig{};
  return SPIFFS.remove(configFilePath);
}

bool loadConfig()
{
  if (!SPIFFS.exists(configFilePath))
  {
    Serial.println("[CONFIG] File not found, using default config");
    return true;
  }

  File file = SPIFFS.open(configFilePath, FILE_READ);
  if (!file)
  {
    Serial.println("[CONFIG] Failed to open file for reading, using default config");
    return true;
  }

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("[CONFIG] Failed to parse file, using default config");
    file.close();
    return true;
  }

  strlcpy(globalConfig.wifiSsid, doc["wifiSsid"] | "", sizeof(globalConfig.wifiSsid));
  strlcpy(globalConfig.wifiPass, doc["wifiPass"] | "", sizeof(globalConfig.wifiPass));
  strlcpy(globalConfig.iotCorePrivateKey, doc["iotCorePrivateKey"] | "", sizeof(globalConfig.iotCorePrivateKey));

  file.close();
  return false;
}

void setupConfig()
{
  uint64_t chipId = ESP.getEfuseMac();
  uint32_t highBytesChipId = (uint32_t)(chipId >> 16); // High 4 bytes
  //uint16_t lowBytesChipId = (uint16_t)chipId; // Low 2 bytes
  snprintf(globalConfig.deviceId, sizeof(globalConfig.deviceId), "%08X", highBytesChipId);
  snprintf(globalConfig.deviceName, sizeof(globalConfig.deviceName), "LAMP_%08X", highBytesChipId);

  SPIFFS.begin(true);
}
