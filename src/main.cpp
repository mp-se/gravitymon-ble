#include <Arduino.h>
#include <ble.hpp>
#include <blescanner.hpp>

BleSender* myBleSender = nullptr;
char chip[20];

void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_NOTICE, &Serial, true);

  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&chip[0], sizeof(chip), "%6x", chipId);
  Log.notice(F("Main: Started setup for %s." CR), &chip[0]);

#if defined(SERVER_TILT) || defined(SERVER_TILT_PRO) || defined(SERVER_GRAVITYMON) || defined(SERVER_GRAVITYMON_EXT) || defined(SERVER_EDDY)
  Log.info(F("Running in BROADCAST mode!" CR));
  myBleSender = new BleSender();
  myBleSender->init();
#endif

#if defined(CLIENT)
  bleScanner.init();
#endif

  Log.info(F("Setup completed!" CR));
}

int loopCounter = 0;

void loop() {
#if defined(SERVER_TILT)
  Log.info(F("Tilt beacon started" CR));
  String color("pink");
  myBleSender->sendTiltData(color, 41.234, 1.23456, false);
#endif

#if defined(SERVER_TILT_PRO)
  Log.info(F("Tilt PRO beacon started" CR));
  String color("green");
  myBleSender->sendTiltData(color, 41.234, 1.23456, true);
#endif

#if defined(SERVER_EDDY)
  Log.info(F("EddyStone beacon started" CR));
  myBleSender->sendEddyStone(3.34567, 42.12345, 1.234567, 89.76543);
#endif

#if defined(SERVER_GRAVITYMON)
  Log.info(F("Gravitymon beacon started" CR));
  String param(loopCounter);
  String json("{\"name\":\"my_device_name\",\"ID\": \"" + String(chip) + "\",\"token\":\"my_token\",\"interval\":" + String(loopCounter) + ",\"temperature\":20.2,\"temp_units\":\"C\",\"gravity\":1.05,\"angle\":34.45,\"battery\":3.85,\"RSSI\":-76.2}");
  myBleSender->sendGravitymonData(json);
  
  int counter = 0;

  while(counter < 100) { // 10 seconds
    delay(100);
    Serial.printf(".");

    if(myBleSender->isGravitymonDataSent()) {
      Log.info(F(CR "BLE beacon has been read" CR));
      break;
    }
    counter ++;   
  }

  myBleSender->stopAdvertising();
#endif

#if defined(SERVER_GRAVITYMON_EXT)
  Log.info(F("Gravitymon extended beacon started" CR));
  String param(loopCounter);
  // String json("{\"name\":\"my_device_name\",\"ID\": \"" + String(chip) + "\",\"token\":\"my_token\",\"interval\":" + String(loopCounter) + ",\"temperature\":20.2,\"temp_units\":\"C\",\"gravity\":1.05,\"angle\":34.45,\"battery\":3.85,\"RSSI\":-76.2}");
  String json("{\"n\":\"my_device_name\",\"I\":\"" + String(chip) + "\",\"to\":\"my_token\",\"i\":" + String(loopCounter) + ",\"t\":20.2,\"u\":\"C\",\"g\":1.05,\"a\":34.45,\"b\":3.85,\"R\":-76.2}");
  myBleSender->sendGravitymonDataExtended(json);

  Log.info(F("Length of payload %d" CR), json.length());

  int counter = 0;

  while(counter < 100) { // 10 seconds
    delay(100);
    Serial.printf(".");

    if(myBleSender->isGravitymonDataSent()) {
      Log.info(F(CR "BLE beacon has been read" CR));
      break;
    }
    counter ++;   
  }

  myBleSender->stopAdvertising();
#endif

#if defined(SERVER_TILT) || defined(SERVER_TILT_PRO) || defined(SERVER_GRAVITYMON) || defined(SERVER_GRAVITYMON_EXT) || defined(SERVER_EDDY)
  delay(10000);
#endif

#if defined(CLIENT)
  bleScanner.scan();
  bleScanner.waitForScan();
#endif

  loopCounter++;
}

// EOF