#include <Arduino.h>
#include <ArduinoLog.h>
#include <ble.hpp>

BleSender* myBleSender;

//#define BROADCAST_TILT
//#define BROADCAST_TILT_PRO
#define BROADCAST_GRAVITYMON

void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_NOTICE, &Serial, true);

  myBleSender = new BleSender();
  Log.info(F("Setup completed!" CR));
}

int loopCounter = 0;

void loop() {
#if defined(BROADCAST_TILT)
  Log.info(F("Tilt beacon started" CR));
  String color("red");
  myBleSender->sendTiltData(color, 41.234, loopCounter*1.1, false);
  delay(5000);
#endif

#if defined(BROADCAST_TILT_PRO)
  Log.info(F("Tilt PRO beacon started" CR));
  String color("red");
  myBleSender->sendTiltData(color, 41.234, 1.2345, true);
  delay(5000);
#endif

#if defined(BROADCAST_GRAVITYMON)
  Log.info(F("Gravitymon beacon started" CR));
  String param(loopCounter);
  String json("{\"name\":\"my_device_name\",\"ID\": \"112233\",\"token\":\"my_token\",\"interval\":" + String(loopCounter) + ",\"temperature\":20.2,\"temp_units\":\"C\",\"gravity\":1.05,\"angle\":34.45,\"battery\":3.85,\"RSSI\":-76.2}");
  myBleSender->sendGravitymonData(json);
  
  int counter = 0;

  while(counter < 100) {
    delay(100);
    Serial.printf(".");

    if(myBleSender->isGravitymonDataSent()) {
      Log.info(F(CR "BLE beacon has been read" CR));
      break;
    }
    counter ++;   
  }
#endif

  loopCounter++;
}

// EOF