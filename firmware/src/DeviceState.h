#ifndef DEVICESTATE_H_
#define DEVICESTATE_H_

struct DeviceState
{
  // Connected to WiFi
  bool online = false;
  // MQTT Connected
  bool connected = false;

  // BLE Ready
  bool bleRunning = false;
  bool bleSetup = false;

  int lampState = 0x0;
  int lampBrightness = 0;

  bool hasWifiChanges = false;
  bool hasStateChanges = false;

  unsigned long lastResetButtonTime = 0;
};

extern DeviceState globalState;

void updateLampState(bool on);
void updateLampBrightness(int brightness);

#endif // DEVICESTATE_H_