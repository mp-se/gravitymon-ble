#include <Arduino.h>
#include <ble.hpp>

BleSender* myBleSender;

//#define USE_TILT
//#define USE_TILT_PRO
#define USE_GRAVITYMON

void setup() {
  Serial.begin(115200);
  myBleSender = new BleSender();
  Serial.println("Setup completed!");
}

int loopCounter = 0;

void loop() {
#if defined(USE_TILT)
  String color("red");
  myBleSender->sendTiltData(color, 41.234, loopCounter*1.1, false);
  delay(5000);
#endif

#if defined(USE_TILT_PRO)
  String color("red");
  myBleSender->sendTiltData(color, 41.234, 1.2345, true);
  delay(5000);
#endif

#if defined(USE_GRAVITYMON)
  Serial.printf("BLE beacon started\n");
  String param(loopCounter);
  String json("{\"name\":\"my_device_name\",\"ID\": \"112233\",\"token\":\"my_token\",\"interval\":" + String(loopCounter) + ",\"temperature\":20.2,\"temp_units\":\"C\",\"gravity\":1.05,\"angle\":34.45,\"battery\":3.85,\"RSSI\":-76.2}");
  myBleSender->sendGravitymonData(json);
  
  int counter = 0;

  while(counter < 100) {
    delay(100);
    Serial.printf(".");

    if(myBleSender->isGravitymonDataSent()) {
      Serial.printf("\nBLE beacon has been read\n");
      break;
    }
    counter ++;   
  }
#endif

  loopCounter++;
}

// EOF