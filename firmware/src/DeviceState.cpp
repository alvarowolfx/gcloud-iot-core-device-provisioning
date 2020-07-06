#include "DeviceState.h"
#include <Arduino.h>

void updateLampState(bool on)
{
  globalState.lampState = on ? HIGH : LOW;
  if (globalState.lampBrightness == 0 && on)
  {
    globalState.lampBrightness = 100;
  }
  globalState.hasStateChanges = true;
}

void updateLampBrightness(int brightness)
{
  if (brightness >= 0 && brightness <= 100)
  {
    globalState.lampBrightness = brightness;
    if (globalState.lampBrightness == 0)
    {
      globalState.lampState = LOW;
    }
    else
    {
      globalState.lampState = HIGH;
    }
    globalState.hasStateChanges = true;
  }
}
