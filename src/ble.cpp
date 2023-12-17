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
#if defined(ESP32) && !defined(ESP32S2)

#include <ble.hpp>
#include <string>

// Tilt UUID variants and data format, based on tilt-sim
//
// https://github.com/spouliot/tilt-sim
//
// Tilt data format is described here. Only SG and Temp is transmitted over BLE.
// https://kvurd.com/blog/tilt-hydrometer-ibeacon-data-format/

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
      Log.info(F("onConnect" CR)); 
    };

    void onDisconnect(NimBLEServer* pServer) {
      Log.info(F("onDisconnect" CR)); 
#if defined(CONFIG_BT_NIMBLE_EXT_ADV)
      BLEDevice::startAdvertising(0);
      // BLEDevice::startAdvertising(1);
#endif
    };
};

static ServerCallbacks myServerCallbacks;

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
 private:
  volatile bool _isRead = false;

 public:
    void clearReadFlag() { _isRead = false; }
    bool isRead() { return _isRead; }

    void onRead(NimBLECharacteristic* pCharacteristic){
      Log.info(F("onRead" CR)); 
      _isRead = true;
    }
};

static CharacteristicCallbacks myCharCallbacks;

#if defined(CONFIG_BT_NIMBLE_EXT_ADV)
class AdvertisingCallbacks: public NimBLEExtAdvertisingCallbacks {
    void onStopped(NimBLEExtAdvertising* pAdv, int reason, uint8_t inst_id) {
      Log.info(F("onStopped" CR)); 
    }

    void onScanRequest(NimBLEExtAdvertising *pAdv, uint8_t inst_id, NimBLEAddress addr) {
      Log.info(F("onScanRequest" CR)); 
    }
};

static AdvertisingCallbacks myAdvertisingCallbacks;
#endif

BleSender::BleSender() {}

void BleSender::init() {
  if(_advertising != nullptr)
    return;

  BLEDevice::init("gravitymon");
  _advertising = BLEDevice::getAdvertising();

  // boost power to maximum, these might be changed once battery life using BLE
  // has been tested.
#if defined(ESP32C3_REV1)
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P6);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P6);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P6);
#else
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
#endif
}

void BleSender::sendTiltData(String& color, float tempF, float gravSG, bool tiltPro) {
  Log.info(F("Starting tilt data transmission" CR));

  if (!color.compareTo("red"))
    _uuidTilt = BLEUUID::fromString("A495BB10-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("green"))
    _uuidTilt = BLEUUID::fromString("A495BB20-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("black"))
    _uuidTilt = BLEUUID::fromString("A495BB30-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("purple"))
    _uuidTilt = BLEUUID::fromString("A495BB40-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("orange"))
    _uuidTilt = BLEUUID::fromString("A495BB50-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("blue"))
    _uuidTilt = BLEUUID::fromString("A495BB60-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("yellow"))
    _uuidTilt = BLEUUID::fromString("A495BB70-C5B1-4B44-B512-1370F02D74DE");
  else  // if (_color.compareTo("pink"))
    _uuidTilt = BLEUUID::fromString("A495BB80-C5B1-4B44-B512-1370F02D74DE");

  Log.info(F("Using UUID %s" CR), _uuidTilt.toString().c_str());

  uint16_t gravity = gravSG * 1000;  // SG * 1000 or SG * 10000 for Tilt Pro/HD
  uint16_t temperature = tempF;      // Deg F _or_ Deg F * 10 for Tilt Pro/HD

  if (tiltPro) { // Experimental, have not figured out how the receiver recognise between standard and Pro/HD
    gravity = gravSG * 10000;
    temperature = tempF * 10;
  }

#if defined(CONFIG_BT_NIMBLE_EXT_ADV)
  BLEBeacon beacon = BLEBeacon();
  beacon.setManufacturerId(0x4C00);
  beacon.setProximityUUID(_uuidTilt);
  beacon.setMajor(temperature);
  beacon.setMinor(gravity);

  NimBLEExtAdvertisement advData = NimBLEExtAdvertisement();
  advData.setFlags(0x04);  
  advData.setManufacturerData(beacon.getData());
  advData.setLegacyAdvertising(true);

  if(_advertising->setInstanceData(0, advData) && _advertising->start(0)) {
    Log.info(F("Started advertising for #0" CR));
  } else {
    Log.info(F("Failed to start advertising for #0" CR));
  }

  delay(_sendTime);
  _advertising->stop(0);
#else  
  BLEBeacon beacon = BLEBeacon();
  beacon.setManufacturerId(0x4C00);
  beacon.setProximityUUID(_uuidTilt);
  beacon.setMajor(temperature);
  beacon.setMinor(gravity);

  BLEAdvertisementData advData = BLEAdvertisementData();
  advData.setFlags(0x04);  
  advData.setManufacturerData(beacon.getData());
  _advertising->setAdvertisementData(advData);

  _advertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON); 
  _advertising->start();
  delay(_sendTime);
  _advertising->stop();
#endif
}

void BleSender::sendGravitymonData(String& payload) {
  Log.info(F("Starting gravitymon data transmission" CR));
#if defined(CONFIG_BT_NIMBLE_EXT_ADV)
  if (!_server) { 
    _server = BLEDevice::createServer();
    _server->setCallbacks(&myServerCallbacks);
    _service = _server->createService(SERV_UUID);
    _characteristic = _service->createCharacteristic(CHAR_UUID, NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::BROADCAST );
    _characteristic->setCallbacks(&myCharCallbacks);
    _service->start();

    _advertising->setCallbacks(&myAdvertisingCallbacks);

    NimBLEExtAdvertisement advData = NimBLEExtAdvertisement();
    advData.setFlags(0x04);  
    advData.setLegacyAdvertising(true);
    advData.setConnectable(true);
    advData.setScannable(true);
    advData.setName("gravitymon");
    advData.setCompleteServices16({_service->getUUID()});

    // Extended advertising stops after disconnect so its started in the advertising callback again

    if(_advertising->setInstanceData(0, advData) && _advertising->start(0)) {
      Log.info(F("Started advertising for #0" CR));
    } else {
      Log.info(F("Failed to start advertising for #0" CR));
    }
  }
#else
  if (!_server) {
    _server = BLEDevice::createServer();
    _server->setCallbacks(&myServerCallbacks);

    _service = _server->createService(SERV_UUID);
    _characteristic = _service->createCharacteristic(CHAR_UUID, NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::BROADCAST );
    _characteristic->setValue("{}");
    _characteristic->setCallbacks(&myCharCallbacks);
    _service->start();

    _advertising->addServiceUUID(_service->getUUID());
    _advertising->setScanResponse(true);
    _advertising->setMinPreferred(0x06);
    _advertising->setMaxPreferred(0x12);
    _advertising->start();
  }
#endif
  myCharCallbacks.clearReadFlag();
  _characteristic->setValue(payload);
  Log.info(F("Characteristic defined, ready for reading!" CR));
}

void BleSender::sendGravitymonDataExtended(String& payload) {
  Log.info(F("Starting gravitymon extended data transmission" CR));
#if defined(CONFIG_BT_NIMBLE_EXT_ADV)
  if (!_server) { 
    _server = BLEDevice::createServer();
    _server->setCallbacks(&myServerCallbacks);
    _service = _server->createService(SERV_UUID);
    _service->start();

    _advertising->setCallbacks(&myAdvertisingCallbacks);
  }

  // This is for extended advertising and sending a smaller payload. 
  // Can be read by another ESP32 but not my iPhone or Windows computer, see Client target.
  // Requires active scanning to be able to detect this.

  NimBLEExtAdvertisement extData(BLE_HCI_LE_PHY_1M, BLE_HCI_LE_PHY_2M);

  extData.setScannable(true);
  extData.setConnectable(false);
  extData.setServiceData(NimBLEUUID(SERV_UUID), std::string(payload.c_str()));
  extData.setShortName("gravitymon");
  extData.setCompleteServices16({NimBLEUUID(SERV_UUID)});

  if(_advertising->setInstanceData(0, extData) && _advertising->start(0)) {
    Log.info(F("Started advertising for #0" CR));
  } else {
    Log.info(F("Failed to start advertising for #0" CR));
  }

  Log.info(F("Extended advertising defined, ready for reading!" CR));
#else
  #warning "Extended advertising is not supported on this target"
  Log.error(F("Extended advertising is not supported on this target!" CR));
#endif
}

bool BleSender::isGravitymonDataSent() {
  return myCharCallbacks.isRead();
}

#endif  // ESP32 && !ESP32S2
