#ifndef DEVICE_CONFIG_H_
#define DEVICE_CONFIG_H_

#include <FS.h>
#include <SPIFFS.h>
#include "DeviceState.h"

struct DeviceConfig
{

  // your network SSID (name)
  char wifiSsid[50] = "";
  // your network password (use for WPA)
  char wifiPass[50] = "";

  char serverHost[100] = "";
  char serverPath[100] = "";

  /* Values not save on storage */
  char deviceId[24] = "";
  char deviceName[24] = "";
  uint resetPin = 2;
  uint lightPin = 13;
  uint lightLedcChannel = 0;
  uint lightLedcFrequency = 5000;
};

extern DeviceConfig globalConfig;

void setupConfig();
bool loadConfig();
bool saveConfig();
bool clearConfig();

#endif // DEVICE_CONFIG_H_