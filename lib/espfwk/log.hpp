/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#ifndef SRC_LOG_HPP_
#define SRC_LOG_HPP_

#include <ArduinoLog.hpp>
#include <espframework.hpp>

#define ERR_FILENAME "/error.log"
#define ERR_FILENAME2 "/error2.log"
#define ERR_FILEMAXSIZE 2048

class SerialDebug {
 private:
  uint32_t _serialSpeed;
 public:
  explicit SerialDebug(const uint32_t serialSpeed = 115200L, bool autoBegin = true, uint8_t tx = -1, uint8_t rx = -1);

  void begin(Print* p);
  uint32_t getSerialSpeed() { return _serialSpeed; }
  static Logging* getLog() { return &Log; }
};

void printTimestamp(Print* _logOutput, int _logLevel);
void printNewline(Print* _logOutput);

void writeErrorLog(const char* format, ...);
void dumpErrorLog1();
void dumpErrorLog2();

#define EspSerial Serial

#endif  // SRC_LOG_HPP_

// EOF
