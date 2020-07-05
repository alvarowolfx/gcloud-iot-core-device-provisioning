
#include <Arduino.h>
#include <time.h>

#include "NTP.h"
#include "DeviceConfig.h"

void setupNtp()
{
  configTime(0, 0, globalConfig.ntpPrimary, globalConfig.ntpSecondary);
  Serial.println("[NTP] Waiting on time sync...");
  while (time(nullptr) < 1510644967)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

time_t getNow()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("[NTP] Failed to obtain time");
    return 0;
  }

  time_t now;
  time(&now);

  return now;
}

String getTimestamp()
{
  time_t now = getNow();
  struct tm *dt;
  dt = localtime(&now);
  char buffer[30];
  strftime(buffer, sizeof(buffer), " %H:%M:%S %d/%m/%Y", dt);

  return String(buffer);
}

void timeTask(void *p)
{
  while (1)
  {
    Serial.println("[NTP] " + getTimestamp());
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
