/*
MIT License

Copyright (c) 2021-2023 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#ifndef SRC_BLE_HPP_
#define SRC_BLE_HPP_

#if defined(ESP32) && !defined(ESP32S2)

#include <Arduino.h>
#include <ArduinoLog.h>

#undef LOG_LEVEL_ERROR
#undef LOG_LEVEL_INFO

#include <NimBLEBeacon.h>
#include <NimBLEDevice.h>

#define SERV_UUID "180A" // Device Information (Payload)
#define SERV2_UUID "1801" // Generic Access (Device Name)
#define CHAR_UUID "2AC4" // Object Properties (Payload)
#define EDDY_UUID "FEAA" // Google EddyStone advertisement

class BleSender {
 private:
  const int _sendTime = 500; // ms

#if defined(CONFIG_BT_NIMBLE_EXT_ADV)
  NimBLEExtAdvertising* _advertising = nullptr;
#else
  BLEAdvertising* _advertising = nullptr;
#endif  
  BLEServer* _server = nullptr;
  BLEService* _service = nullptr;
  BLECharacteristic* _characteristic = nullptr;
  BLEUUID _uuidTilt;

 public:
  explicit BleSender();

  void init();

  void sendTiltData(String& color, float tempF, float gravSG, bool tiltPro);

  void sendEddystone(float battery, float tempC, float gravity, float angle);

  void sendGravitymonData(String& payload);
  void sendGravitymonDataExtended(String& payload);
  bool isGravitymonDataSent();

  void clearReadFlags();
  void stopAdvertising();
};

#endif  // ESP32 && !ESP32S2
#endif  // SRC_BLE_HPP_
