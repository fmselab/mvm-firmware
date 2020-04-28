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
   // The datasheet does not specify how the CRC bits are added up 
   // but the firmware code doesn't seem to check them anyway.
   // Here the 16-bit words are XORed together and the 4 LSBs are used.

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

   // Init the Q coefficients;
   switch(m_model)
    {
     case pp001DS:
      m_q[0]=15; m_q[1]=17; m_q[2]=7; m_q[3]=5; m_q[4]=7, m_q[5]=21;
      break;
     case pp002GS:
      m_q[0]=14; m_q[1]=16; m_q[2]=8; m_q[3]=6; m_q[4]=7, m_q[5]=22;
      break;
     case pp002DS:
      m_q[0]=16; m_q[1]=18; m_q[2]=6; m_q[3]=4; m_q[4]=7, m_q[5]=22;
      break;
     case pp005GS:
      m_q[0]=16; m_q[1]=17; m_q[2]=6; m_q[3]=5; m_q[4]=7, m_q[5]=21;
      break;
     case pp005DS:
      m_q[0]=17; m_q[1]=19; m_q[2]=5; m_q[3]=3; m_q[4]=7, m_q[5]=22;
      break;
     case pp015GS:
      m_q[0]=16; m_q[1]=17; m_q[2]=6; m_q[3]=5; m_q[4]=7, m_q[5]=22;
      break;
     case pp015AS:
      m_q[0]=16; m_q[1]=17; m_q[2]=6; m_q[3]=5; m_q[4]=7, m_q[5]=22;
      break;
     case pp015DS:
      m_q[0]=17; m_q[1]=19; m_q[2]=5; m_q[3]=3; m_q[4]=7, m_q[5]=22;
      break;
     case pp030AS:
      m_q[0]=17; m_q[1]=18; m_q[2]=5; m_q[3]=4; m_q[4]=7, m_q[5]=22;
      break;
     case pp030GS:
      m_q[0]=17; m_q[1]=18; m_q[2]=5; m_q[3]=4; m_q[4]=7, m_q[5]=22;
      break;
     case pp030DS:
      m_q[0]=18; m_q[1]=21; m_q[2]=4; m_q[3]=1; m_q[4]=7, m_q[5]=22;
      break;
    }
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
      << now.tv_sec << ":" << now.tv_nsec/1000000 << " - tick:"
      << FW_TEST_tick << " - ";
  
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
  int dt, d1, d2, psim;
  int64_t off, sens;

  switch (cmd)
   {
    case 0x0:

      dt = ((m_treading*100.) - 2000.)/m_prom[6]*(1<<m_q[5]);
      d2 =  dt + m_prom[5]*(1<<m_q[4]);

      // Here we invert this datasheet recipe:
      // Difference between actual and reference temperature
      // dT = D2 - T REF = D2 - C5 * 2^Q5
      //  TEMP Measured temperature
      // TEMP=20°C+dT*TEMPSENS=2000+dT*C6/2^Q6
      // Calculate Temperature Compensated Pressure
      // OFF Offset at actual temperature
      //OFF=OFF T1 +TCO * dT=C2*2^Q2 +(C4*dT)/2^Q4
      //SENS Sensitivity at actual temperature
      //SENS=SENS T1 +TCS*dT=C1*2^Q1 +(C3*dT)/2^Q3 
      //P Temperature Compensated Pressure
      //P=D1*SENS-OFF=(D1*SENS/2^21 -OFF)/2^15

      psim = m_preading*10000;
      off = m_prom[1]*(1<<m_q[1]) + (m_prom[4] * dt)/(1<<m_q[3]);
      sens = m_prom[1]*(1<<m_q[0]) + (m_prom[3] * dt)/(1<<m_q[2]);
      d1 = (psim*(1<<15) + off) * (1<<21)/sens;
      if (rlength < 3) return I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
      if (m_want_to_read_pressure)
       {
        rbuffer[0] = (d1&0xff0000) >> 16;
        rbuffer[1] = (d1&0xff00) >> 8;
        rbuffer[2] = (d1&0xf) >> 8;
       } else {
        rbuffer[0] = (d2&0xff0000) >> 16;
        rbuffer[1] = (d2&0xff00) >> 8;
        rbuffer[2] = (d2&0xf) >> 8;
       }
      msg << "Read " << (m_want_to_read_pressure ? "pressure" : "temperature")
          << " ADC "  << std::hex << std::showbase << cmd 
          << ") command received. Returning [" << std::hex 
          << std::setfill('0') << rbuffer[0] << "][" << rbuffer[1] 
          << "][" << rbuffer[2] << "].";
      ret = 3;
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
      m_preading = FW_TEST_qtl_double.value(m_name + "_pressure",FW_TEST_tick);
      m_want_to_read_pressure = true;
      msg << "D1 Setup (" << std::hex << std::showbase << cmd 
          << ") command received.";
      break;
    case 0x50:
    case 0x52:
    case 0x54:
    case 0x56:
    case 0x58:
      m_treading = FW_TEST_qtl_double.value(m_name + "_temperature",FW_TEST_tick);
      if (std::isnan(m_treading))
       {
        m_treading = FW_TEST_qtl_double.value("env_temperature",FW_TEST_tick);
       }
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
  int ret = -1;

  timespec now;
  ::clock_gettime(CLOCK_REALTIME, &now);
  std::ostringstream msg;
  msg << I2C_DEVICE_module_name << " - ADS1115 - " << m_name << " - "
      << now.tv_sec << ":" << now.tv_nsec/1000000 << " - tick:"
      << FW_TEST_tick << " - ";
  
  if ((cmd >= 0) && (cmd <= 3))
   {
    if (wlength >= 2)
     {
      m_reg[cmd] = ((rbuffer[0] << 8) | rbuffer[1]);
      if (cmd == CONFIG_REG)
       {
        m_cur_mux = ((m_reg[CONFIG_REG] & 0x7000) >> 12);
        m_cur_gain = ((m_reg[CONFIG_REG] & 0x0e00) >> 9);
        m_vmax = (6.144/(1<<m_cur_gain));
        if (m_cur_gain > 5) m_cur_gain = 5;
        if ((m_reg[CONFIG_REG] & 0x8000) || // One-shot conversion ?
            (m_reg[CONFIG_REG] & 0x0100))   // Continuous conversion ?
         {
          m_reg[CONVERSION_REG] = 0;
          if (m_cur_mux == 4)
           {
            m_o2_concentration = FW_TEST_qtl_double.value("o2_concentration",FW_TEST_tick);
            m_reg[CONVERSION_REG] = (m_o2_concentration - m_o2_sensor_calib_m)/
                                    m_o2_sensor_calib_q;
            msg << "O2 concentration: " << m_o2_concentration << "%";
           }
          else if ((m_cur_mux >= 5) && (m_cur_mux <= 7))
           {
            m_voltage_ref = FW_TEST_qtl_double.value("voltage_ref",FW_TEST_tick);
            uint16_t vrefs;
            if (m_voltage_ref <= 0) vrefs = 0;
            else if (m_voltage_ref > m_vmax) vrefs = 0xffff;
            else vrefs = 0xffff*(m_voltage_ref/m_vmax);
            msg << " Voltage reference: " << m_voltage_ref << " V";
            
            if (m_cur_mux == 6)
             {
              m_voltage_12v = FW_TEST_qtl_double.value("voltage_12v",FW_TEST_tick);
              msg << " 12V voltage: " << m_voltage_12v << " V";
              m_reg[CONVERSION_REG] = (m_voltage_12v/(2.5*5.)) * vrefs;
             }
            else if (m_cur_mux == 7)
             {
              m_voltage_5v = FW_TEST_qtl_double.value("voltage_5v",FW_TEST_tick);
              msg << " 5V voltage: " << m_voltage_5v << " V";
              m_reg[CONVERSION_REG] = (m_voltage_5v/(2.5*2.)) * vrefs;
             }
            else m_reg[CONVERSION_REG] = vrefs;
           }
          
          msg << " == [" << std::hex << std::setfill('0')
              << m_reg[CONVERSION_REG] << std::dec;
          if (m_reg[CONFIG_REG] & 0x8000) m_reg[CONFIG_REG] &= 0x7fff;
         }
       }
     }
    if (rlength >= 2)
     {
      rbuffer[0] = (m_reg[cmd]&0xff00) >> 8;
      rbuffer[1] = (m_reg[cmd]&0xff);
      msg << "Read register " << std::hex << std::showbase << cmd
          << ". Returning [" << std::hex << std::setfill('0')
          << rbuffer[0] << "][" << rbuffer[1] << "]." << std::dec;
      ret = 2;
     }
   }
  else
   {
    msg << "UNKNOWN (" << std::hex << std::showbase << cmd 
        << ") command received.";
    ret = I2C_DEVICE_SIMUL_UNKNOWN_CMD;
   }
  m_dbg.DbgPrint(DBG_CODE, DBG_INFO, msg.str().c_str());
  return ret;
};

