//
// File: Wire.h
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 15-May-2020 Initial version.
//
// Description:
// Interface to simulated I2C devices at the Arduino 'TwoWire' class level.
// Needed as the HAL layer seems to be disappearing from the MVM Firmware.
//

#ifndef _MVM_FW_TEST_WIRE_H
#define _MVM_FW_TEST_WIRE_H

#include <vector>
#include <stdint.h> // cstdint is c++11

class simulate_i2c_devices;
 
class TwoWire
{
  public:
    TwoWire(simulate_i2c_devices &devs): m_sim_devs(devs),
                                         m_w_started(false), m_r_started(true)
     {
      m_w_buf.reserve(16);
      m_r_buf.reserve(16);
      m_r_it = m_r_buf.begin();
     }
    ~TwoWire() {}
  
    typedef std::vector<uint8_t> buffer_t;

    bool begin(int sda=-1, int scl=-1, uint32_t frequency=0) { return true; }
  
    uint8_t receive() { return read(); }
    uint8_t read();
    void send(uint8_t c); 
    void write(uint8_t c) { send(c); }
  
    uint8_t requestFrom(uint16_t address, uint8_t size, bool sendStop);
  
    void beginTransmission(uint8_t address);
    void beginTransmission(uint16_t address)
     {
      beginTransmission(static_cast<uint8_t>(address));
     }
    void beginTransmission(int address)
     {
      beginTransmission(static_cast<uint8_t>(address));
     }
  
    uint8_t endTransmission(bool sendStop);
    uint8_t endTransmission(void) { return endTransmission(false); }

    bool busy() { return false; }
  private:
    simulate_i2c_devices &m_sim_devs;
    buffer_t m_w_buf;
    buffer_t m_r_buf;
    buffer_t::const_iterator m_r_it;
    uint8_t m_cur_addr;
    bool m_w_started;
    bool m_r_started;
};

extern TwoWire Wire;

#endif /* defined _MVM_FW_TEST_WIRE_H */
