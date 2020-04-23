/* Stub implementation of the Arduino 'Wire' class. */

#ifndef _MVM_FW_TEST_SERIAL_H
#define _MVM_FW_TEST_SERIAL_H

#include <cstdio>
#include <cstdint>

#include "WString.h"
#include "esp32-hal-uart.h"

#define DEC 10
#define HEX 16

class SerialImpl
{
  public:
  SerialImpl();
  ~SerialImpl();

  bool available();

  void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1,
             int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL);
  size_t println(const String &);
  size_t println(const char[]);
  size_t println(unsigned long, int = DEC);
  size_t print(const char[]);
  size_t print(const String &);

  String readStringUntil(char end);
};

extern SerialImpl Serial;

#endif /* defined _MVM_FW_TEST_SERIAL_H */
