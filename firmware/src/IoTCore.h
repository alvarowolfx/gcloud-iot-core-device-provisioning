#ifndef MQTT_H_
#define MQTT_H_

#include <Arduino.h>

void mqttTask(void *p);
void setupCloudIoT();
void publishState(String data);

#endif // MQTT_H_