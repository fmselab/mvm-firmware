/* Stub implementation of the Arduino 'Wire' class. */
/* Needed to work around some leftover cruft in the Firmware code. */

#ifndef _MVM_FW_TEST_WIRE_H
#define _MVM_FW_TEST_WIRE_H

#include <cstdint>

class WireImpl
{
  public:
  WireImpl() {}
  ~WireImpl() {}

  bool begin(int sda=-1, int scl=-1, uint32_t frequency=0) { return true; }

  uint8_t receive() { return 0; }
  uint8_t read() { return 0; } 
  void send(uint8_t c) {} 
  void write(uint8_t c) {}

  uint8_t requestFrom(uint16_t address, uint8_t size, bool sendStop) { return 0;}

  void beginTransmission(uint16_t address) {}
  void beginTransmission(uint8_t address) {}
  void beginTransmission(int address) {}

  uint8_t endTransmission(bool sendStop) { return 0; }
  uint8_t endTransmission(void) { return 0; }

};

extern WireImpl Wire;

#endif /* defined _MVM_FW_TEST_WIRE_H */
