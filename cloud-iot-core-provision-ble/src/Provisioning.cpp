#include "Provisioning.h"
#include "DeviceConfig.h"

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <ArduinoJson.h>

// Nordic UART Service
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
BLECharacteristic *pTxCharacteristic = NULL;

class ProvisioningCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
      Serial.println("*********");
      Serial.print("New value: ");
      Serial.print(value.c_str());
      Serial.println();
      Serial.println("*********");
      pCharacteristic->setValue(value);

      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, value);
      if (error)
      {
        Serial.println("Failed to parse json");
        return;
      }
      bool hasChanges = false;
      bool hasConfigChanges = false;
      bool hasWifiChanges = false;
      if (doc.containsKey("wifiSsid"))
      {
        strlcpy(globalConfig.wifiSsid, doc["wifiSsid"] | "", sizeof(globalConfig.wifiSsid));
        hasConfigChanges = true;
        hasWifiChanges = true;
      }
      if (doc.containsKey("wifiPass"))
      {
        strlcpy(globalConfig.wifiPass, doc["wifiPass"] | "", sizeof(globalConfig.wifiPass));
        hasConfigChanges = true;
        hasWifiChanges = true;
      }
      if (doc.containsKey("iotCorePrivateKey"))
      {
        strlcpy(globalConfig.iotCorePrivateKey, doc["iotCorePrivateKey"] | "", sizeof(globalConfig.iotCorePrivateKey));
        hasConfigChanges = true;
      }
      if (doc.containsKey("power"))
      {
        globalState.lampState = doc["power"] | LOW;
        if (globalState.lampBrightness == 0)
        {
          globalState.lampBrightness = 100;
        }
        hasChanges = true;
      }
      if (doc.containsKey("brightness"))
      {
        globalState.lampBrightness = doc["brightness"] | 0;
        hasChanges = true;
      }
      if (hasConfigChanges)
      {
        saveConfig();
      }
      if (hasChanges)
      {
        globalState.hasChanges = hasChanges;
      }
      if (hasWifiChanges)
      {
        globalState.hasWifiChanges = hasWifiChanges;
      }
    }
  }
};

void setupProvisioning()
{
  BLEDevice::init(globalConfig.deviceName);

  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      NIMBLE_PROPERTY::NOTIFY);

  pTxCharacteristic->createDescriptor("2902");

  pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      NIMBLE_PROPERTY::WRITE);

  pRxCharacteristic->setCallbacks(new ProvisioningCallbacks());

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);
}

void startProvisioningServer()
{
  // Start service
  pService->start();
  BLEDevice::startAdvertising();
  globalState.bleReady = true;
}

void stopProvisioningServer()
{
  Serial.println("Stop advertising!");
  BLEDevice::stopAdvertising();
}

void updateBleStatus(String value)
{
  const char *data = value.c_str();
  pTxCharacteristic->setValue((uint8_t *)data, value.length());
  pTxCharacteristic->notify();
  delay(10);

  Serial.print("Sending response over ble : ");
  Serial.println(value);
}
