# End to End Example of Provisioning an IoT device via BLE

This repository show an end to end onboarding flow of an IoT Device using an ESP32 with an BLE interface, a Flutter App using that BLE interface, a backend to talk IoT Core to provision and send commands and a Cloud Function to ingest data from IoT Core to Firebase.

Steps:

1. Device stars with no Wifi Credentials and Private Key to connect to IoT Core.
2. User can press a button on the device to put it on provisioning mode and this starts a BLE service that accepts commands and the configuration parameters.
3. An app searchs the device over BLE
4. The app generates an public/private ECC key pair, sends the sends the private key over BLE to the device and register the device using the `api` server sending device ID and the public key.
5. After device receives configurations, it tries to connect to WiFi and to Google Cloud IoT Core over MQTT ( the devices needs to be registered on IoT Core). The device will start to report changes over MQTT.
6. App disconnect over BLE and sends commands over internet to IoT Core thought the `api` server.

️ ⚠️️️️**THIS IS A WORK IN PROGRESS** ⚠️

## Table of Contents

- [Getting Started](#getting-started)
  - [Node Setup](#node-setup)
  - [Firebase Setup](#firebase-setup)
  - [Google Cloud Setup](#google-cloud-tools-and-project)
  - [Deploy Backend](#deploy-backend)
  - [Flutter Setup](#flutter-setup)
- Project Structure
  - Cloud Function - `functions` folder
    - `processDeviceData` : Save data sent by the device on Firebase
    - Built with NodeJS
  - Provisioning API - `api` folder
    - Provisioning and device association API
    - All communication to register devices on IoT Core and send commands.
    - Built with Golang
  - Flutter App - `app` folder
    - Mobile App built with Flutter

# Getting Started

## Hardware

### BOM - Bill of Materials

- ESP32
  - Any model, I used one with a dedicated Boot button on GPIO0 that is used to trigger provisioning and also reset the device by holding it for 5 seconds or more.

### Upload firmware with PlatfomIO

I recommend installing the Visual Studio Code (VSCode) IDE and the PlatformIO plugin to get started using it. Just follow the step on the link below:

https://platformio.org/platformio-ide

To deploy to the board, just open the `firmware` folder and you can use the “Build” and “Upload” buttons on PlatformIO Toolbar. All libraries and dependencies will be downloaded.

## Google Cloud Setup

- Install gcloud CLI - [Link](https://cloud.google.com/sdk/install)
- Authenticate with Google Cloud:
  - `gcloud auth login`
- Create cloud project — choose your unique project name:
  - `gcloud projects create YOUR_PROJECT_NAME`
- Set current project
  - `gcloud config set project YOUR_PROJECT_NAME`
- Associate with a Firebase Project - [Link](https://console.firebase.google.com/)

### Create IoT Core resources

- Add permissions for IoT Core
  - `gcloud projects add-iam-policy-binding YOUR_PROJECT_NAME --member=serviceAccount:cloud-iot@system.gserviceaccount.com --role=roles/pubsub.publisher`
- Create PubSub topic for device data:
  - `gcloud beta pubsub topics create devices-inbound`
- Create PubSub subscription for device data:
  - `gcloud beta pubsub subscriptions create --topic devices-inbound devices-inbound-subscription`
- Create device registry:
  - `gcloud beta iot registries create weather-station-registry --region us-central1 -event-notification-config=topic=projects/YOUR_PROJECT_NAME/topics/devices-inbound`

## Deploy Backend

### Deploy API on Cloud Run

TODO:

### Deploy Cloud Functions to ingest IoT Core data

- Run on the command line:

```
cd functions
npm run deploy
```

## Flutter Setup

- Follow the guide on their [website](https://flutter.dev/docs/get-started/install).
- Run the following command to make sure it's all good.

```shell
flutter doctor
```

#### References

- https://cloud.google.com/iot-core
- https://github.com/GoogleCloudPlatform/google-cloud-iot-arduino
- https://github.com/h2zero/NimBLE-Arduino/
