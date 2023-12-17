/*
MIT License

Copyright (c) 2023 Magnus

Based on code created by John Beeler on 5/12/18 (Tiltbridge project).

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

#ifndef SRC_BLESCANNER_HPP_
#define SRC_BLESCANNER_HPP_

#include <ArduinoJson.h>
#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>

#include <string>

class BleDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice *advertisedDevice) override;
};

class BleClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) override;
};

enum TiltColor {
  None = -1,
  Red = 0,
  Green = 1,
  Black = 2,
  Purple = 3,
  Orange = 4,
  Blue = 5,
  Yellow = 6,
  Pink = 7
};

struct TiltData {
  float tempF = 0;
  float gravity = 0;
  int txPower = 0;
  int rssi = 0;
  uint32_t timeStamp = 0;
};

struct GravitymonData {
  NimBLEAddress address;
  String data;
  bool doConnect;
};

const auto NO_TILT_COLORS = 8;
const auto NO_GRAVITYMON = 4;

class BleScanner {
 public:
  BleScanner();
  void init();
  void deInit();

  bool scan();
  bool waitForScan();

  TiltColor proccesTiltBeacon(const std::string &advertStringHex,
                              const int8_t &currentRSSI);

  void processGravitymonBeacon(NimBLEAddress address);
  void processGravitymonExtBeacon(NimBLEAddress address, const std::string &payload);

  TiltData getTiltData(TiltColor col) { return _tilt[col]; }

  int getGravitymonCount() { return _gravitymonCount; }
  String getGravitymonData(int idx) { return _gravitymon[idx].data; }

 private:
  const int _scanTime = 5;

  BLEScan *_bleScan = nullptr;

  BleDeviceCallbacks *_deviceCallbacks = nullptr;
  BleClientCallbacks *_clientCallbacks = nullptr;

  // Tilt related data
  TiltData _tilt[NO_TILT_COLORS];

  // Gravitymon related data
  int _gravitymonCount = 0;
  GravitymonData _gravitymon[NO_GRAVITYMON];

  TiltColor uuidToTiltColor(std::string uuid);
  bool connectGravitymonDevice(int idx);
};

extern BleScanner bleScanner;

#endif  // SRC_BLESCANNER_HPP_
