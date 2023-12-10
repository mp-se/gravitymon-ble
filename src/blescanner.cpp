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

#include <ArduinoLog.h>

#include <blescanner.hpp>

BleScanner bleScanner;

const char* TILT_COLOR_RED_UUID = "a495bb10c5b14b44b5121370f02d74de";
const char* TILT_COLOR_GREEN_UUID = "a495bb20c5b14b44b5121370f02d74de";
const char* TILT_COLOR_BLACK_UUID = "a495bb30c5b14b44b5121370f02d74de";
const char* TILT_COLOR_PURPLE_UUID = "a495bb40c5b14b44b5121370f02d74de";
const char* TILT_COLOR_ORANGE_UUID = "a495bb50c5b14b44b5121370f02d74de";
const char* TILT_COLOR_BLUE_UUID = "a495bb60c5b14b44b5121370f02d74de";
const char* TILT_COLOR_YELLOW_UUID = "a495bb70c5b14b44b5121370f02d74de";
const char* TILT_COLOR_PINK_UUID = "a495bb80c5b14b44b5121370f02d74de";

void BleDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
  // Check if we have a gravitymon beacon to process
  if (advertisedDevice->haveName() && advertisedDevice->getName() == "gravitymon") {
      Log.notice(F("BLE : Processing gravitymon beacon" CR));
      bleScanner.processGravitymonBeacon(advertisedDevice->getAddress());
      //NimBLEDevice::getScan()->stop();
  }

  // Check if we have a tilt beacon to process
  if (advertisedDevice->getManufacturerData().length() >= 24) {
    if (advertisedDevice->getManufacturerData()[0] == 0x4c &&
        advertisedDevice->getManufacturerData()[1] == 0x00 &&
        advertisedDevice->getManufacturerData()[2] == 0x02 &&
        advertisedDevice->getManufacturerData()[3] == 0x15) {
      Log.notice(F("BLE : Advertised iBeacon Device: %s" CR),
                  advertisedDevice->toString().c_str());

      bleScanner.proccesTiltBeacon(advertisedDevice->getManufacturerData(),
                                   advertisedDevice->getRSSI());
    }
  }
}

void BleClientCallbacks::onConnect(NimBLEClient* client) {
  Log.notice(F("BLE : Client connected"));
  //client->updateConnParams(120,120,0,60);
}

void BleScanner::processGravitymonBeacon(NimBLEAddress address) {
  Log.notice(F("BLE : Advertised gravitymon device: %s" CR), address.toString().c_str());

  if(_gravitymonCount < NO_GRAVITYMON) {
    _gravitymon[_gravitymonCount].address = address;
    _gravitymonCount++;
  }
  else {
    Log.notice(F("BLE : Max devices reached - no more devices available." CR));
  }
}

bool BleScanner::connectGravitymonDevice(int idx) {
  Log.notice(F("BLE : Connecting to gravitymon device: %s" CR), _gravitymon[idx].address.toString().c_str());

  NimBLEClient* client = nullptr;

  if(NimBLEDevice::getClientListSize()) {
    client = NimBLEDevice::getClientByPeerAddress(_gravitymon[idx].address);
    if(client) {
      if(!client->connect(_gravitymon[idx].address, false)) {
        Log.warning(F("BLE : Reconnect failed." CR));
        return false;
      }
      Log.notice(F("BLE : Reconnected with client." CR));
    } else {
      client = NimBLEDevice::getDisconnectedClient();
    }
  }

  if(!client) {
    if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
      Log.warning(F("BLE : Max clients reached - no more connections available" CR));
      return false;
    }

    client = NimBLEDevice::createClient();
    Log.notice(F("BLE : New client created." CR));
    client->setClientCallbacks(_clientCallbacks, false);

    // Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
    // These settings are safe for 3 clients to connect reliably, can go faster if you have less
    // connections. Timeout should be a multiple of the interval, minimum is 100ms.
    // Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
    client->setConnectionParams(12,12,0,51);
    client->setConnectTimeout(5);

    if(!client->connect(_gravitymon[idx].address)) {
      NimBLEDevice::deleteClient(client);
      Log.warning(F("BLE : Failed to connect, deleted client." CR));
      return false;
    }
  }

  if(!client->isConnected()) {
    if (!client->connect(_gravitymon[idx].address)) {
      Log.warning(F("BLE : Failed to connect." CR));
      return false;
    }
  }

  Log.notice(F("BLE : Connected to: %s, RSSI: %d" CR), client->getPeerAddress().toString().c_str(), client->getRssi());

  NimBLERemoteService* srv = nullptr;
  NimBLERemoteCharacteristic* chr = nullptr;

  srv = client->getService("180A");
  
  if(srv) {
    chr = srv->getCharacteristic("2900");

    if(srv) {
      if(chr->canRead()) {
        _gravitymon[idx].data = chr->readValue();
        Log.notice(F("uuid=%s, value=%s" CR), chr->getUUID().toString().c_str(), _gravitymon[idx].data.c_str());
      }
    } else {
      client->disconnect();
      Log.warning(F("BLE : Unable to find service 180A!" CR));
      return false;
    }
  }

  Log.notice(F("BLE : Done reading data from gravitymon device!" CR));
  client->disconnect();
  return true;
}

BleScanner::BleScanner() {
  _deviceCallbacks = new BleDeviceCallbacks();
  _clientCallbacks = new BleClientCallbacks();
}

void BleScanner::init() {
  NimBLEDevice::init("");
  _bleScan = NimBLEDevice::getScan(); 
  _bleScan->setAdvertisedDeviceCallbacks(_deviceCallbacks);
  _bleScan->setMaxResults(0);
  _bleScan->setActiveScan(false);
  _bleScan->setInterval(
      97);  // Select prime numbers to reduce risk of frequency beat pattern
            // with ibeacon advertisement interval
  _bleScan->setWindow(37);  // Set to less or equal setInterval value. Leave
                            // reasonable gap to allow WiFi some time.
  scan();
}

void BleScanner::deInit() {
  waitForScan();
  NimBLEDevice::deinit();
}

bool BleScanner::scan() {
  if (!_bleScan)
    return false;
  
  if(_bleScan->isScanning())
    return true;
  
  _bleScan->clearResults();
  _gravitymonCount = 0;

  Log.notice(F("BLE : Starting passive scan." CR));

  if (_bleScan->start(_scanTime, nullptr, true)) {
    return true;
  }

  Log.notice(F("BLE : Scan failed to start." CR));
  return false;
}

bool BleScanner::waitForScan() {
  if (!_bleScan) return false;

  while (_bleScan->isScanning()) {
    delay(100);
  }

  for(int i = 0; i < _gravitymonCount; i ++) {
    uint32_t start = millis();
    connectGravitymonDevice(i);
    Log.info(F("Connected with device %d, took %d ms" CR), i, millis()-start);
  }

  return true;
}

TiltColor BleScanner::proccesTiltBeacon(const std::string& advertStringHex,
                                        const int8_t& currentRSSI) {
  TiltColor color;

  // Check that this is an iBeacon packet
  if (advertStringHex[0] != 0x4c || advertStringHex[1] != 0x00 ||
      advertStringHex[2] != 0x02 || advertStringHex[3] != 0x15)
    return TiltColor::None;

  // The advertisement string is the "manufacturer data" part of the following:
  // Advertised Device: Name: Tilt, Address: 88:c2:55:ac:26:81, manufacturer
  // data: 4c000215a495bb40c5b14b44b5121370f02d74de005004d9c5
  // 4c000215a495bb40c5b14b44b5121370f02d74de005004d9c5
  // ????????iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiittttggggXR
  // **********----------**********----------**********
  char hexCode[3] = {'\0'};
  char colorArray[33] = {'\0'};
  char tempArray[5] = {'\0'};
  char gravityArray[5] = {'\0'};
  char txPowerArray[3] = {'\0'};

  for (int i = 4; i < advertStringHex.length(); i++) {
    snprintf(hexCode, sizeof(hexCode), "%.2x", advertStringHex[i]);
    // Indices 4 - 19 each generate two characters of the color array
    if ((i > 3) && (i < 20)) {
      strncat(colorArray, hexCode, 2);
    }
    // Indices 20-21 each generate two characters of the temperature array
    if (i == 20 || i == 21) {
      strncat(tempArray, hexCode, 2);
    }
    // Indices 22-23 each generate two characters of the sp_gravity array
    if (i == 22 || i == 23) {
      strncat(gravityArray, hexCode, 2);
    }
    // Index 24 contains the tx_pwr (which is used by recent tilts to indicate
    // battery age)
    if (i == 24) {
      strncat(txPowerArray, hexCode, 2);
    }
  }

  color = uuidToTiltColor(colorArray);
  if (color == TiltColor::None) {
    return TiltColor::None;
  }

  uint16_t temp = std::strtoul(tempArray, nullptr, 16);
  uint16_t gravity = std::strtoul(gravityArray, nullptr, 16);
  uint8_t txPower = std::strtoul(txPowerArray, nullptr, 16);

  Log.notice(F("BLE : Tilt data received %d, %d, %d" CR), temp, gravity,
             txPower);

  _tilt[color].gravity = gravity;
  _tilt[color].tempF = temp;
  _tilt[color].txPower = txPower;
  _tilt[color].rssi = currentRSSI;
  _tilt[color].timeStamp = millis();

  return color;
}

TiltColor BleScanner::uuidToTiltColor(std::string uuid) {
  if (uuid == TILT_COLOR_RED_UUID) {
    return TiltColor::Red;
  } else if (uuid == TILT_COLOR_GREEN_UUID) {
    return TiltColor::Green;
  } else if (uuid == TILT_COLOR_BLACK_UUID) {
    return TiltColor::Black;
  } else if (uuid == TILT_COLOR_PURPLE_UUID) {
    return TiltColor::Purple;
  } else if (uuid == TILT_COLOR_ORANGE_UUID) {
    return TiltColor::Orange;
  } else if (uuid == TILT_COLOR_BLUE_UUID) {
    return TiltColor::Blue;
  } else if (uuid == TILT_COLOR_YELLOW_UUID) {
    return TiltColor::Yellow;
  } else if (uuid == TILT_COLOR_PINK_UUID) {
    return TiltColor::Pink;
  }
  return TiltColor::None;
}

// EOF