//
// File: Wire.cpp
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

#include "Wire.h"

#include "simulate_i2c_devices.h"
 
uint8_t
TwoWire::read()
{
  uint8_t ret = 0;

  if (m_r_it != m_r_buf.end()) ret = (*m_r_it);
  ++m_r_it;
  return ret;
}

void
TwoWire::send(uint8_t c) 
{
  m_w_buf.push_back(c);
}

uint8_t
TwoWire::requestFrom(uint16_t address, uint8_t size, bool sendStop)
{
  uint8_t ret = 0;
  m_r_buf.reserve(size);
  ret = m_sim_devs.exchange_message(m_cur_addr, NULL, 0, &(m_r_buf[0]), size,
                                    sendStop);

  m_r_it = m_r_buf.begin();
  return ret;
}
  
void
TwoWire::beginTransmission(uint8_t address)
{
  m_cur_addr = address;
  m_w_started = true;
}

uint8_t
TwoWire::endTransmission(bool sendStop)
{
  if (!m_w_started) return -1;

  uint8_t ret = 0;
  ret = m_sim_devs.exchange_message(m_cur_addr, &(m_w_buf[0]), m_w_buf.size(),
                                    NULL, 0, sendStop);

  m_w_buf.clear();

  return ret;
}
