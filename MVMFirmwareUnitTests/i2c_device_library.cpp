//
// File: i2c_device_library.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 27-Apr-2020 Initial version.
//
// Description:
// Implementation of simulated I2C devices. As the time for development
// is short only commands that are actually used ade implemented. We try
// not to depend on any information coded in the Firmware itself though.
//

#include <iomanip>
#include <sstream>
#include <ctime> // Real time clock.

#include "i2c_device_library.h"

void
mvm_fw_unit_test_TE_MS5525DSO::m_init_prom()
{
   // How the CRC bit are added up is not specified in the datasheet,
   // but the firmware code doesn't seem to check them anyway.
   // Here the 16-bit words are XORed together and 4 LSBs are used.

   if (!FW_TEST_main_config.get_ushort_array(m_name + "_prom", m_prom,
                                          sizeof(m_prom)/sizeof(m_prom[0])))
    {
     // No config. use default values. 
     m_prom[0] = 0xbac0;  // 'Reserved'
     m_prom[1] = 0x8e32;  // C1 - Pressure sensitivity
     m_prom[2] = 0x9a31;  // C2 - Pressure offset
     m_prom[3] = 0x9dc9;  // C3 - Pressure sens. Temp.coeff.
     m_prom[4] = 0x7353;  // C4 - Pressure offs. Temp.coeff.
     m_prom[5] = 0x749e;  // C5 - Reference Temperature
     m_prom[6] = 0x559d;  // C6 - Temp. Coeff of Temp
     m_prom[7] = 0x0000;
    }

   uint16_t crc = 0;
   for (int i = 0; i < 8; ++i)
    {
     crc ^= m_prom[i];
    }

   m_prom[7] &= 0xfff0;
   m_prom[7] |= crc&0xf;
}

int
mvm_fw_unit_test_TE_MS5525DSO::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  int ret = -1;

  timespec now;
  ::clock_gettime(CLOCK_REALTIME, &now);
  std::ostringstream msg;
  msg << I2C_DEVICE_module_name << " - MS5525DSO - " << m_name << " - "
      << now.tv_sec << ":" << now.tv_nsec/1000000 << " - ";
  
  if ((cmd >= 0xa0) && (cmd <= 0xae))
   {
    if (rlength < 2) return I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
    uint16_t pval = m_prom[cmd - 0xa0];
    // big-endian response
    rbuffer[0] = (pval & 0xff00) >> 8;
    rbuffer[1] =  pval & 0xff;
    msg << "Read PROM " << std::hex << std::showbase << cmd
        << ") command received. Returning [" << std::hex << std::setfill('0')
        << rbuffer[0] << "][" << rbuffer[1] << "].";
    m_dbg.DbgPrint(DBG_CODE, DBG_INFO, msg.str().c_str());
    return 2;
   }

  double dreading = 0;
  uint32_t reading=0xffffffff;
  switch (cmd)
   {
    case 0x0:

      if (m_want_to_read_pressure)
       {
        dreading = FW_TEST_qtl_double.value(m_name + "_pressure",FW_TEST_tick);
       }
      else
       {
        //Read temperature.
        dreading = FW_TEST_qtl_double.value(m_name + "_temperature",FW_TEST_tick);
        if (std::isnan(dreading))
         {
          dreading = FW_TEST_qtl_double.value("env_temperature",FW_TEST_tick);
         }
       }
      // XXX Add recipe for goung from double to hex.
      msg << "Read ADC (" << std::hex << std::showbase << cmd 
          << ") command received.";
      break;
    case 0x1e:
      msg << "RESET (" << std::hex << std::showbase << cmd 
          << ") command received.";
      ret = 0;
      break;
    case 0x40:
    case 0x42:
    case 0x44:
    case 0x46:
    case 0x48:
      m_want_to_read_pressure = true;
      msg << "D1 Setup (" << std::hex << std::showbase << cmd 
          << ") command received.";
      break;
    case 0x50:
    case 0x52:
    case 0x54:
    case 0x56:
    case 0x58:
      m_want_to_read_pressure = false;
      msg << "D2 Setup (" << std::hex << std::showbase << cmd 
          << ") command received.";
      break;

    default:
      msg << "UNKNOWN (" << std::hex << std::showbase << cmd 
          << ") command received.";
      ret = I2C_DEVICE_SIMUL_UNKNOWN_CMD;
      break;
   }
  m_dbg.DbgPrint(DBG_CODE, DBG_INFO, msg.str().c_str());
  return ret;
};

int
mvm_fw_unit_test_SENSIRION_SFM3019::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  return -1;
};

int
mvm_fw_unit_test_TI_ADS1115::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  return -1;
};

