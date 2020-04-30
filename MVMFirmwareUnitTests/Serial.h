/* Stub implementation of the Arduino 'Serial' class, */
/* as it seems to be used directly outside of the HW abstraction */

#ifndef _MVM_FW_TEST_SERIAL_H
#define _MVM_FW_TEST_SERIAL_H

#include <cstdio>
#include <stdint.h>

#include "WString.h"

#define DEC 10
#define HEX 16
#define SERIAL_8N1 0x800001c

class SerialImpl
{
  public:
  SerialImpl() : m_ttys(0) {}
  ~SerialImpl() {}

  bool available();

  void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1,
             int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL);
  size_t println(const String &);
  size_t println(const char[]);
  size_t println(unsigned long, int = DEC);
  size_t print(const char[]);
  size_t print(const String &);

  void set_ttys(FILE *ttys) { m_ttys = ttys; }

  String readStringUntil(char end);

  private:
    FILE *m_ttys;
};

extern SerialImpl Serial;

#endif /* defined _MVM_FW_TEST_SERIAL_H */
