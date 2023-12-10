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
#include <NimBLEBeacon.h>
#include <NimBLEDevice.h>

class BleSender {
 private:
  BLEAdvertising* _advertising = NULL;
  BLEServer* _server = NULL;
  BLEService* _service = NULL;
  BLECharacteristic* _characteristic = NULL;
  BLEUUID _uuid;

 public:
  explicit BleSender();

  void sendTiltData(String& color, float tempF, float gravSG, bool tiltPro);

  void sendGravitymonData(String& payload);
  bool isGravitymonDataSent();
};

#endif  // ESP32 && !ESP32S2
#endif  // SRC_BLE_HPP_
