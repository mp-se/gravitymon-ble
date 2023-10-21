#include <Arduino.h>
#include <ble.hpp>

BleSender* myBleSender;

//#define USE_TILT
//#define USE_TILT_PRO
#define USE_GRAVITYMON

void setup() {
  Serial.begin(115200);
  myBleSender = new BleSender();

#if defined(USE_GRAVITYMON)
  String json("");
  myBleSender->sendGravitymonData(json);
#endif

  Serial.println("Setup completed!");
}

int loopCounter = 0;

void loop() {
#if defined(USE_TILT)
  String color("red");
  myBleSender->sendTiltData(color, 41.234, 1.2345, false);
#endif

#if defined(USE_TILT_PRO)
  String color("red");
  myBleSender->sendTiltData(color, 41.234, 1.2345, true);
#endif

#if defined(USE_GRAVITYMON)
  String param(loopCounter);
  String json("{\"name\":\"my_device_name\",\"ID\": \"01234567\",\"token\":\"my_token\",\"interval\":" + String(loopCounter) + ",\"temperature\":20.2,\"temp_units\":\"C\",\"gravity\":1.05,\"angle\":34.45,\"battery\":3.85,\"RSSI\":-76.2,}");
  myBleSender->sendGravitymonData(json);
  delay(100);
  Serial.printf("BLE beacon has been read: %s\n", myBleSender->isGravitymonRead() ? "true" : "false");
#endif

  delay(2000);
  loopCounter++;
}

// EOF