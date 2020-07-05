#ifndef PROVISIONING_H_
#define PROVISIONING_H_

#include <FS.h>
#include <SPIFFS.h>
#include "DeviceState.h"
#include "DeviceConfig.h"

extern DeviceConfig globalConfig;
extern DeviceState globalState;

void startProvisioningServer();
void setupProvisioning();
void stopProvisioningServer();
void updateBleStatus(String value);

#endif // PROVISIONING_H_