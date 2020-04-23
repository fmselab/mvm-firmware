/* Stub implementation of the Arduino 'Wire' class. */

#ifndef _MVM_FW_TEST_WIRE_H
#define _MVM_FW_TEST_WIRE_H

#include <cstdint>

class WireImpl
{
  public:
  WireImpl();
  ~WireImpl();

  bool begin(int sda=-1, int scl=-1, uint32_t frequency=0); // returns true, if successful init of i2c bus

  uint8_t receive();
  uint8_t read();
  void send(uint8_t c);
  void write(uint8_t c);

  uint8_t requestFrom(uint16_t address, uint8_t size, bool sendStop);

  void beginTransmission(uint16_t address);
  void beginTransmission(uint8_t address);
  void beginTransmission(int address);

  uint8_t endTransmission(bool sendStop);
  uint8_t endTransmission(void);

};

extern WireImpl Wire;

#endif /* defined _MVM_FW_TEST_WIRE_H */
