/*
MIT License

Copyright (c) 2021-2025 Magnus

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
#if defined(CHAMBER)

#include <ble_chamber_scan.hpp>
#include <cstdio>
#include <log.hpp>
#include <memory>
#include <string>
#include <utils.hpp>
#include <vector>

BleScanner bleScanner;

constexpr auto SERV_UUID = "180A";
constexpr auto SERV2_UUID = "1801";
constexpr auto CHAR_UUID = "2AC4";

void BleDeviceCallbacks::onResult(
    const NimBLEAdvertisedDevice *advertisedDevice) {
  // Log.notice(F("BLE : %s,%s %d" CR),
  //            advertisedDevice->getAddress().toString().c_str(),
  //            advertisedDevice->getName().c_str(),
  //             advertisedDevice->getManufacturerData().length());

  if (advertisedDevice->getName() == "gravitymon") {
    bool eddyStone = false;

    // Print out the advertised services
    for (int i = 0; i < advertisedDevice->getServiceDataCount(); i++) {
      // Check if we have a gravitymon eddy stone beacon.
      for (int i = 0; i < advertisedDevice->getServiceDataCount(); i++) {
        if (advertisedDevice->getServiceDataUUID(i).toString() ==
            "0xfeaa") {  // id for eddystone beacon
          eddyStone = true;
        }
      }
    }

    if (eddyStone) {
      Log.notice(F("BLE : Processing gravitymon eddy stone device" CR));
      bleScanner.processGravitymonEddystoneBeacon(
          advertisedDevice->getAddress(), advertisedDevice->getPayload());
    }

    return;
  }

  if (advertisedDevice->getManufacturerData().length() >= 24) {
    if (advertisedDevice->getManufacturerData()[0] == 0x4c &&
        advertisedDevice->getManufacturerData()[1] == 0x00 &&
        advertisedDevice->getManufacturerData()[2] == 0x03 &&
        advertisedDevice->getManufacturerData()[3] == 0x15) {
      Log.notice(
          F("BLE : Advertised iBeacon GRAVMON device: %s" CR),
          advertisedDevice->getAddress().toString().c_str());

      bleScanner.proccesGravitymonBeacon(
          advertisedDevice->getManufacturerData(),
          advertisedDevice->getAddress());
    }
  }
}

void BleScanner::proccesGravitymonBeacon(const std::string &advertStringHex,
                                         NimBLEAddress address) {
  const char *payload = advertStringHex.c_str();

  float battery;
  float temp;
  float gravity;
  float angle;
  uint32_t chipId;

  if (*(payload + 4) == 'G' && *(payload + 5) == 'R' && *(payload + 6) == 'A' &&
      *(payload + 7) == 'V') {
    Log.info(F("BLE : Found gravitymon beacon." CR));

    chipId = (*(payload + 12) << 24) | (*(payload + 13) << 16) |
             (*(payload + 14) << 8) | *(payload + 15);
    angle = static_cast<float>((*(payload + 16) << 8) | *(payload + 17)) / 100;
    battery =
        static_cast<float>((*(payload + 18) << 8) | *(payload + 19)) / 1000;
    gravity =
        static_cast<float>((*(payload + 20) << 8) | *(payload + 21)) / 10000;
    temp = static_cast<float>((*(payload + 22) << 8) | *(payload + 23)) / 1000;

    char chip[20];
    snprintf(chip, sizeof(chip), "%06x", chipId);

    std::unique_ptr<MeasurementBaseData> gravityData;
    gravityData.reset(new GravityData(MeasurementSource::BleBeacon, chip, "",
                                      "", temp, gravity, angle, battery, 0, 0,
                                      0));

    Log.info(F("BLE : Update data for gravitymon %s." CR),
             gravityData->getId());
    myMeasurementList.updateData(gravityData);
  }
}

void BleScanner::processGravitymonEddystoneBeacon(
    NimBLEAddress address, const std::vector<uint8_t> &payload) {
  //                                                                      <--------------
  //                                                                      beacon
  //                                                                      data
  //                                                                      ------------>
  // 0b 09 67 72 61 76 69 74 79 6d 6f 6e 02 01 06 03 03 aa fe 11 16 aa fe 20 00
  // 0c 8b 10 8b 00 00 30 39 00 00 16 2e

  float battery;
  float temp;
  float gravity;
  float angle;
  uint32_t chipId;

  battery = static_cast<float>((payload[25] << 8) | payload[26]) / 1000;
  temp = static_cast<float>((payload[27] << 8) | payload[28]) / 1000;
  gravity = static_cast<float>((payload[29] << 8) | payload[30]) / 10000;
  angle = static_cast<float>((payload[31] << 8) | payload[32]) / 100;
  chipId = (payload[33] << 24) | (payload[34] << 16) | (payload[35] << 8) |
           (payload[36]);

  char chip[20];
  snprintf(chip, sizeof(chip), "%06x", chipId);

  std::unique_ptr<MeasurementBaseData> gravityData;
  gravityData.reset(new GravityData(MeasurementSource::BleEddyStone, chip, "",
                                    "", temp, gravity, angle, battery, 0, 0,
                                    0));

  Log.info(F("BLE : Update data for gravitymon %s." CR), gravityData->getId());
  myMeasurementList.updateData(gravityData);
}

BleScanner::BleScanner() { _deviceCallbacks = new BleDeviceCallbacks(); }

void BleScanner::init() {
  NimBLEDevice::init("");
  _bleScan = NimBLEDevice::getScan();
  _bleScan->setScanCallbacks(_deviceCallbacks);
  _bleScan->setMaxResults(0);
  _bleScan->setActiveScan(_activeScan);

  _bleScan->setInterval(
      97);  // Select prime numbers to reduce risk of frequency beat pattern
            // with ibeacon advertisement interval
  _bleScan->setWindow(37);  // Set to less or equal setInterval value. Leave
                            // reasonable gap to allow WiFi some time.
  scan();
}

void BleScanner::deInit() { NimBLEDevice::deinit(); }

bool BleScanner::scan() {
  if (!_bleScan) return false;

  if (_bleScan->isScanning()) return true;

  _bleScan->clearResults();

  Log.notice(F("BLE : Starting %s scan." CR),
             _activeScan ? "ACTIVE" : "PASSIVE");
  _bleScan->setActiveScan(_activeScan);
  _bleScan->start(_scanTime * 1000, false, true);

  Log.notice(F("BLE : Scanning completed." CR));
  return true;
}

#endif  // CHAMBER
