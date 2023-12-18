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
#include <NimBLEUtils.h>

BleScanner bleScanner;

const char* TILT_COLOR_RED_UUID = "a495bb10c5b14b44b5121370f02d74de";
const char* TILT_COLOR_GREEN_UUID = "a495bb20c5b14b44b5121370f02d74de";
const char* TILT_COLOR_BLACK_UUID = "a495bb30c5b14b44b5121370f02d74de";
const char* TILT_COLOR_PURPLE_UUID = "a495bb40c5b14b44b5121370f02d74de";
const char* TILT_COLOR_ORANGE_UUID = "a495bb50c5b14b44b5121370f02d74de";
const char* TILT_COLOR_BLUE_UUID = "a495bb60c5b14b44b5121370f02d74de";
const char* TILT_COLOR_YELLOW_UUID = "a495bb70c5b14b44b5121370f02d74de";
const char* TILT_COLOR_PINK_UUID = "a495bb80c5b14b44b5121370f02d74de";

const char* SERV_UUID = "180A";
const char* SERV2_UUID = "1801";
const char* CHAR_UUID = "2AC4";

void BleDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {

  // Log.notice(F("BLE : %s" CR), advertisedDevice->toString().c_str());
  // Log.notice(F("BLE : %s %s" CR), advertisedDevice->getName().c_str(), advertisedDevice->getAddress().toString().c_str());

  if( advertisedDevice->getName() == "gravitymon") {
    bool eddyStone = false;

    // Check if we have a gravitymon eddy stone beacon.
    for( int i = 0; i < advertisedDevice->getServiceDataCount(); i ++)
      if( advertisedDevice->getServiceDataUUID(i).toString() == "0xfeaa") // id for eddystone beacon
        eddyStone = true;

    if(eddyStone) {
      Log.notice(F("BLE : Processing gravitymon eddy stone beacon" CR));
      bleScanner.processGravitymonEddystoneBeacon(advertisedDevice->getAddress(), advertisedDevice->getPayload());
    } else if( advertisedDevice->getServiceData(NimBLEUUID(SERV2_UUID)) == "gravitymon_ext") {
      Log.notice(F("BLE : Processing gravitymon extended beacon" CR));
      bleScanner.processGravitymonExtBeacon(advertisedDevice->getAddress(), advertisedDevice->getServiceData(NimBLEUUID(SERV_UUID)));
    } else {
      Log.notice(F("BLE : Processing gravitymon beacon (connect with device)" CR));
      bleScanner.processGravitymonBeacon(advertisedDevice->getAddress());
    }

    return;
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

void BleScanner::processGravitymonEddystoneBeacon(NimBLEAddress address, const uint8_t *payload) {

  //                                                                      <-------------- beacon data ------------>      
  // 0b 09 67 72 61 76 69 74 79 6d 6f 6e 02 01 06 03 03 aa fe 11 16 aa fe 20 00 0c 8b 10 8b 00 00 30 39 00 00 16 2e

  payload = payload + 23;

  /*
  uint8_t buf[50];
  BLEUtils u;
  u.buildHexData(&buf[0], payload, 14);
  Log.notice(F("BLE : eddy=%s" CR), &buf[0]);
  */

  float battery;
  float temp;
  float gravity;
  float angle;
  uint32_t chipId;

  battery = static_cast<float>((*(payload+2)<<8) | *(payload+3)) / 1000; 
  temp = static_cast<float>((*(payload+4)<<8) | *(payload+5)) / 1000; 
  gravity = static_cast<float>((*(payload+6)<<8) | *(payload+7)) / 10000; 
  angle = static_cast<float>((*(payload+8)<<8) | *(payload+9)) / 100; 
  chipId = (*(payload+10)<<24) | (*(payload+11)<<16) | (*(payload+12)<<8) | *(payload+13); 

  char chip[20];
  snprintf(&chip[0], sizeof(chip), "%6x", chipId);

  DynamicJsonDocument out(500);
  out["name"] = "";
  out["ID"] = &chip[0];
  out["token"] = "";
  out["interval"] = 0;
  out["temperature"] = serialized(String(temp, 2));
  out["temp_units"] = "C";
  out["gravity"] = serialized(String(gravity, 4));
  out["angle"] = serialized(String(angle, 2));
  out["battery"] = serialized(String(battery, 2));
  out["RSSI"] = 0;

  String str;
  str.reserve(500);
  serializeJson(out, str);
  out.clear();

  if(_gravitymonCount < NO_GRAVITYMON) {
    _gravitymon[_gravitymonCount].address = address;
    _gravitymon[_gravitymonCount].data = str;
    _gravitymon[_gravitymonCount].doConnect = false;
    _gravitymonCount++;
  }
  else {
    Log.notice(F("BLE : Max devices reached - no more devices available." CR));
  }
}

void BleScanner::processGravitymonExtBeacon(NimBLEAddress address, const std::string &payload) {
  Log.notice(F("BLE : Advertised gravitymon ext device: %s" CR), address.toString().c_str());

  DynamicJsonDocument out(1000), in(1000);
  DeserializationError err = deserializeJson(in, payload.c_str());

  if (err) {
    Log.error(F("BLE : Failed to parse advertisement json %d" CR), err);
    return;
  }

  // Map the tags to make this a iSpindle json document (using short names to save space in ext advertisement)

  out["name"] = in["n"];
  out["ID"] = in["I"];
  out["token"] = in["to"];
  out["interval"] = in["i"];
  out["temperature"] = in["t"];
  out["temp_units"] = in["u"];
  out["gravity"] = in["g"];
  out["angle"] = in["a"];
  out["battery"] = in["b"];
  out["RSSI"] = in["R"];

  in.clear(); 

  String str;
  str.reserve(1000);
  serializeJson(out, str);
  out.clear();

  if(_gravitymonCount < NO_GRAVITYMON) {
    _gravitymon[_gravitymonCount].address = address;
    _gravitymon[_gravitymonCount].data = str;
    _gravitymon[_gravitymonCount].doConnect = false;
    _gravitymonCount++;
  }
  else {
    Log.notice(F("BLE : Max devices reached - no more devices available." CR));
  }
}

void BleScanner::processGravitymonBeacon(NimBLEAddress address) {
  Log.notice(F("BLE : Advertised gravitymon device: %s" CR), address.toString().c_str());

  if(_gravitymonCount < NO_GRAVITYMON) {
    _gravitymon[_gravitymonCount].address = address;
    _gravitymon[_gravitymonCount].data = "";
    _gravitymon[_gravitymonCount].doConnect = true;
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

  srv = client->getService(SERV_UUID);
  
  if(srv) {
    chr = srv->getCharacteristic(CHAR_UUID);

    if(chr && chr->canRead()) {
      _gravitymon[idx].data = chr->readValue();
      Log.notice(F("uuid=%s, value=%s" CR), chr->getUUID().toString().c_str(), _gravitymon[idx].data.c_str());
    } else {
      client->disconnect();
      Log.warning(F("BLE : Unable to find characteristic %s or not readable!" CR), CHAR_UUID);
      return false;
    }
  } else {
    client->disconnect();
    Log.warning(F("BLE : Unable to find service %s!" CR), SERV_UUID);
    return false;
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
#if defined(ACTIVE_SCAN)
  _bleScan->setActiveScan(true);
#else
  _bleScan->setActiveScan(false);
#endif
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

#if defined(ACTIVE_SCAN)
  Log.notice(F("BLE : Starting ACTIVE scan." CR));
#else
  Log.notice(F("BLE : Starting PASSIVE scan." CR));
#endif

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
    if(_gravitymon[i].doConnect) {
      uint32_t start = millis();
      connectGravitymonDevice(i);
      Log.info(F("Connected with device %d, took %d ms" CR), i, millis()-start);
    }
  
    Log.info(F("Device %d, length=%d, data=%s" CR), i, _gravitymon[i].data.length(), _gravitymon[i].data.c_str());
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
