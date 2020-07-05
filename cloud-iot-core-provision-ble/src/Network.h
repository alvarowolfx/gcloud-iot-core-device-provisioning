#ifndef NETWORK_H_
#define NETWORK_H_

#include "DeviceConfig.h"

bool reconnectWiFi();
void printWifiStatus();
void networkTask(void *p);

#endif // NETWORK_H_