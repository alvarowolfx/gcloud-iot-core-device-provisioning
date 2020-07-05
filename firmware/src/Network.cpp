#include "Network.h"
#include "NTP.h"
#include <WiFi.h>

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

bool connectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.begin(globalConfig.wifiSsid, globalConfig.wifiPass);
  int count = 0;
  bool error = false;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
    count++;
    if (count > 150)
    {
      error = true;
      break;
    }
  }

  return error;
}

bool reconnectWiFi()
{
  //Serial.println("Trying to connect to WiFi...");
  if (WiFi.status() == WL_CONNECTED)
  {
    //Serial.println("Already connected.");
    return false;
  }

  Serial.print("Scanning for network ");
  Serial.print(globalConfig.wifiSsid);
  Serial.println(" ...");

  if (strlen(globalConfig.wifiSsid) == 0)
  {
    Serial.println("Missing SSID");
    return true;
  }

  if (strlen(globalConfig.wifiPass) == 0)
  {
    Serial.println("Missing wifi password");
    return true;
  }

  bool error = connectWifi();
  if (!error)
  {
    printWifiStatus();
  }
  return error;
}

void networkTask(void *p)
{
  while (1)
  {
    bool error = reconnectWiFi();
    bool online = !error;
    if (globalState.online != online)
    {
      globalState.online = online;
      if (error)
      {
        Serial.println("[wifi] Error connecting to WiFi");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
      else
      {
        setupNtp();
        Serial.println("[wifi] Connected to WiFi");
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}