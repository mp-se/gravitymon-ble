/*
MIT License

Copyright (c) 2023 Magnus

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
#include <Arduino.h>

#include <ble_chamber.hpp>
#include <ble_gateway.hpp>
#include <ble_gravitymon.hpp>
#include <ble_pressuremon.hpp>
#include <log.hpp>
#include <utils.hpp>

// #define CLIENT_GRAVITYMON_TILT
// #define CLIENT_GRAVITYMON_TILTPRO
// #define CLIENT_GRAVITYMON_IBEACON
// #define CLIENT_GRAVITYMON_EDDYSTONE
// #define CLIENT_PRESSUREMON_IBEACON
// #define CLIENT_PRESSUREMON_EDDYSTONE

#define CLIENT_CHAMBER_IBEACON

#if defined(PRESSUREMON) || defined(GRAVITYMON) || defined(CHAMBER)
BleSender myBleSender;
#endif

char chip[20];

SerialDebug mySerial;

void setup() {
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&chip[0], sizeof(chip), "%6x", chipId);
  Log.notice(F("Main: Started setup for %s." CR), &chip[0]);

#if defined(PRESSUREMON) || defined(GRAVITYMON)
  Log.info(F("Running in broadcast mode (server)!" CR));
  myBleSender.init();
#endif

#if defined(GATEWAY)
  Log.info(F("Running in listening mode (client)!" CR));
  bleScanner.init();
  bleScanner.setScanTime(5);
  bleScanner.setAllowActiveScan(true);
#endif

  Log.info(F("Setup completed!" CR));
}

void loop() {
  String color;

#if defined(CLIENT_GRAVITYMON_TILT) && defined(GRAVITYMON)
  Log.info(F("Gravitymon TILT server started" CR));
  color = "pink";
  myBleSender.sendTiltData(color, 41.234, 1.23456, false);
  delay(2000);
#endif

#if defined(CLIENT_GRAVITYMON_TILTPRO) && defined(GRAVITYMON)
  Log.info(F("Gravitymon TILT PRO server started" CR));
  color = "green";
  myBleSender.sendTiltData(color, 31.234, 1.12345, true);
  delay(2000);
#endif

#if defined(CLIENT_GRAVITYMON_IBEACON) && defined(GRAVITYMON)
  Log.info(F("Gravitymon iBbeacon server started" CR));
  myBleSender.sendCustomBeaconData(3.34567, 42.12345, 1.234567, 89.76543);
  delay(2000);
#endif

#if defined(CLIENT_GRAVITYMON_EDDYSTONE) && defined(GRAVITYMON)
  Log.info(F("Gravitymon EddyStone server started" CR));
  myBleSender.sendEddystoneData(3.34567, 42.12345, 1.234567, 89.76543);
  delay(2000);
#endif

#if defined(CLIENT_PRESSUREMON_IBEACON) && defined(PRESSUREMON)
  Log.info(F("Pressuremon iBbeacon server started" CR));
  myBleSender.sendCustomBeaconData(3.34567, 42.12345, 1.234567, 89.76543);
  delay(2000);
#endif

#if defined(CLIENT_PRESSUREMON_EDDYSTONE) && defined(PRESSUREMON)
  Log.info(F("Pressuremon EddyStone server started" CR));
  myBleSender.sendEddystoneData(3.34567, 42.12345, 1.234567, 89.76543);
  delay(2000);
#endif

#if defined(CLIENT_CHAMBER_IBEACON) && defined(CHAMBER)
  Log.info(F("Chamber iBbeacon server started" CR));
  myBleSender.sendCustomBeaconData(22.345, 24.765);
  delay(2000);
#endif

#if defined(GATEWAY)
  bleScanner.scan();
  // bleScanner.waitForScan();

  // Process Gravitymon TILT BLE
  for (int i = 0; i < NO_TILT_COLORS; i++) {
    TiltData td = bleScanner.getTiltData((TiltColor)i);

    Log.notice(F("Main: Type=%s, Gravity=%F, Temp=%F." CR),
               bleScanner.getTiltColorAsString((TiltColor)i), td.gravity,
               convertFtoC(td.tempF));
  }

  // Process Gravitymon BLE
  for (int i = 0; i < NO_GRAVITYMON; i++) {
    GravitymonData& gmd = bleScanner.getGravitymonData(i);
    Log.notice(F("Main: Type=%s, Angle=%F Gravity=%F, Temp=%F, Battery=%F, "
                 "Id=%s." CR),
               gmd.type.c_str(), gmd.angle, gmd.gravity, gmd.tempC, gmd.battery,
               gmd.id.c_str());
  }

  // Process Pressuremon BLE
  for (int i = 0; i < NO_PRESSUREMON; i++) {
    PressuremonData& pmd = bleScanner.getPressuremonData(i);
    Log.notice(
        F("Main: Type=%s, Pressure=%F Pressure1=%F, Temp=%F, Battery=%F, "
          "Id=%s." CR),
        pmd.type.c_str(), pmd.pressure, pmd.pressure1, pmd.tempC, pmd.battery,
        pmd.id.c_str());
  }

  // TODO: Add parsing and handling

#endif
}

// EOF
