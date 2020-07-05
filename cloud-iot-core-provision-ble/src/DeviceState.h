#ifndef DEVICESTATE_H_
#define DEVICESTATE_H_

#include "DeviceState.h"

struct DeviceState
{
  // Connected to WiFi
  bool online = false;
  // MQTT Connected
  bool connected = false;

  // BLE Ready
  bool bleReady = false;

  int lampState = 0x0;
  int lampBrightness = 0;

  bool hasWifiChanges = false;
  bool hasChanges = false;
};

extern DeviceState globalState;

#endif // DEVICESTATE_H_