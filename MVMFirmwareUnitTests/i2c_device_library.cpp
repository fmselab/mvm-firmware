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
  m_cmd_0_in = false;
  got_treading = false;
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
      << now.tv_sec << ":" << now.tv_nsec/1000000
      << " - ms (scaled):" << FW_TEST_main_config.get_scaled_ms()
      << " - tick:" << FW_TEST_tick << " - ";

  if ((cmd >= 0xa0) && (cmd <= 0xae))
   {
    if (rlength < 2) return I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
    uint16_t pval = m_prom[(cmd - 0xa0)>>1];
    // big-endian response
    rbuffer[0] = (pval & 0xff00) >> 8;
    rbuffer[1] =  pval & 0xff;
    msg << "Read PROM " << std::hex << std::showbase
        << static_cast<int>(cmd)
        << ") command received. Returning ["
        << std::hex << std::setfill('0') << std::noshowbase
        << static_cast<int>(rbuffer[0]) << "]["
        << static_cast<int>(rbuffer[1]) << "]." << std::dec;
    m_dbg.DbgPrint(DBG_CODE, DBG_VALUE, msg.str().c_str());
    return 2;
   }

  double dreading = 0;
  uint32_t reading=0xffffffff;
  int dt, d1, d2, psim;
  int64_t off, sens;
  const double CMH2O_TO_PSI = 0.014223343307054;

  if ((cmd == 0) && m_cmd_0_in && (rlength > 0))
   {
    m_cmd_0_in = false;
    // dt = (temp[hundredths of C])-2000)/C6*2^Q6:
    dt = ((m_treading*100.) - 2000.)/m_prom[6]*(1<<m_q[5]);
    // d2 = dt + C5*2^Q5
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

    psim = m_preading*(CMH2O_TO_PSI*10000);
    off = static_cast<int64_t>(m_prom[2])*(1<<m_q[1]) +
          (static_cast<int64_t>(m_prom[4]) * dt)/(1<<m_q[3]);
    sens = static_cast<int64_t>(m_prom[1])*(1<<m_q[0]) +
          (static_cast<int64_t>(m_prom[3]) * dt)/(1<<m_q[2]);
    // d1 = ((p * 2^15) + off) * 2^21/SENS
    d1 = (static_cast<uint64_t>(psim)*(1<<15) + off) * (1<<21)/sens;
    if (rlength < 3) return I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
    if (m_want_to_read_pressure)
     {
      rbuffer[0] = (d1&0xff0000) >> 16;
      rbuffer[1] = (d1&0xff00) >> 8;
      rbuffer[2] = (d1&0xff);
     } else {
      rbuffer[0] = (d2&0xff0000) >> 16;
      rbuffer[1] = (d2&0xff00) >> 8;
      rbuffer[2] = (d2&0xff);
     }
    msg << "Read " << (m_want_to_read_pressure ? "pressure" : "temperature")
        << " ADC "  << std::hex << std::showbase << static_cast<int>(cmd)
        << ") command received. Returning [" << std::hex << std::noshowbase
        << std::setfill('0') << static_cast<int>(rbuffer[0])
        << "][" << static_cast<int>(rbuffer[1])
        << "][" << static_cast<int>(rbuffer[2]) << "]." << std::dec;
    ret = 3;
   }
  else
   {
    bool read_pressure = false;
    switch (cmd)
     {
      case 0x0:
        m_cmd_0_in = true;
        ret = 0;
        msg.str("");
        msg.clear(); // No need to log the cmd setup phase
        break;
      case 0x1e:
        msg << "RESET (" << std::hex << std::showbase << static_cast<int>(cmd)
            << ") command received." << std::dec << std::noshowbase;
        ret = 0;
        break;
      case 0x40:
      case 0x42:
      case 0x44:
      case 0x46:
      case 0x48:
        m_preading = FW_TEST_pflow.p_value(m_name,FW_TEST_main_config.get_scaled_ms());
        m_want_to_read_pressure = true;
        msg << "D1 Setup (" << std::hex << std::showbase << static_cast<int>(cmd)
            << ") command received. Pressure == " << std::dec
            << std::noshowbase << m_preading << ".";
        ret = 0;
        read_pressure = true;
        if (got_treading) break;
      case 0x50:
      case 0x52:
      case 0x54:
      case 0x56:
      case 0x58:
        m_treading = FW_TEST_qtl_double.value(m_name + "_temperature",FW_TEST_ms);
        if (std::isnan(m_treading))
         {
          m_treading = FW_TEST_qtl_double.value("env_temperature",FW_TEST_ms);
         }
        got_treading = true;
        if (!read_pressure)
         {
          m_want_to_read_pressure = false;
          msg << "D2 Setup (" << std::hex << std::showbase
              << static_cast<int>(cmd) << ") command received.";
         }
        msg << " Temperature == " << std::dec
            << std::noshowbase << m_treading << ".";
        ret = 0;
        break;

      default:
        msg << "UNKNOWN (" << std::hex << std::showbase << static_cast<int>(cmd)
            << ") command received." << std::dec << std::noshowbase;
        ret = I2C_DEVICE_SIMUL_UNKNOWN_CMD;
        break;
     }
   }
  if (msg.str().length() > 0)
   {
    m_dbg.DbgPrint(DBG_CODE, DBG_VALUE, msg.str().c_str());
   }
  return ret;
};

int
mvm_fw_unit_test_SENSIRION_SFM3019::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  int ret = -1;

  timespec now;
  ::clock_gettime(CLOCK_REALTIME, &now);
  std::ostringstream msg;
  msg << I2C_DEVICE_module_name << " - SFM3019 - " << m_name << " - "
      << now.tv_sec << ":" << now.tv_nsec/1000000
      << " - ms (scaled):" << FW_TEST_main_config.get_scaled_ms()
      << " - tick:" << FW_TEST_tick << " - ";

  if ((wlength <= 0) && ((m_retc.size()*3) >= rlength))
   {
    msg << " Read from stored buffer of size " << m_retc.size();
    ret = 0;
    for (uint8_t *rp = rbuffer; rp <(rbuffer+rlength); rp+=3)
     {
      if ((rp - rbuffer) > (rlength - 3)) break;
      return_word  rw = m_retc.front();
      m_retc.pop_front();
      rw.fill_data_crc(rp);
      ret += 3;
     }
   }
  else if ((wlength <= 0) && (rlength >= 9))
   {
    msg << "Read op ";
    if (!m_m_active)
     {
      ret = I2C_DEVICE_NOT_ACTIVE;
      msg << "received while device not active.";
     }
    else
     {
      m_update_measurement();
      return_word r(m_flow_val);
      ret = 0;
      if (rlength >= 3) { r.fill_data_crc(&(rbuffer[0])); ret += 3; }
      else m_retc.push_back(r);
      r = m_temp_val;
      if (rlength >= 6) { r.fill_data_crc(&(rbuffer[3])); ret += 3; }
      else m_retc.push_back(r);
      r = m_status_word;
      if (rlength >= 6) { r.fill_data_crc(&(rbuffer[6])); ret += 3; }
      else m_retc.push_back(r);
      msg << "flow: [" << std::hex << m_flow_val << "], temp: ["
          << m_temp_val << "], status: [" << m_status_word
          << "] Flow_in == " << m_freading << ".";
     }
   }
  else if ((cmd == 0x36) && (wlength >= 1))
   {
    if (wbuffer[0] == 0x61)
     {
      //Read scale factor
      msg << " Read scale factor";
      if (m_m_active)
       {
        ret = I2C_DEVICE_BUSY;
        msg << " received while device is busy reading out.";
       }
      else
       {
        ret = 0;
        return_word r(m_scale_factor);
        if (rlength >= 3) { r.fill_data_crc(&(rbuffer[0])); ret += 3; }
        else m_retc.push_back(r);
        r = m_offset;
        if (rlength >= 6) { r.fill_data_crc(&(rbuffer[3])); ret += 3; }
        else m_retc.push_back(r);
        r = m_flow_unit;
        if (rlength >= 6) { r.fill_data_crc(&(rbuffer[6])); ret += 3; }
        else m_retc.push_back(r);
       }
     }
    else if (wbuffer[0] == 0x63)
     {
      msg << " Configure averaging - NOP";
      ret = 0;
     }
    else if (wbuffer[0] == 0x77)
     {
      msg << " Enter sleep mode - NOP";
      ret = 0;
     }
    else if ((wbuffer[0] == 0x03) || (wbuffer[0] == 0x08)
                                  || (wbuffer[0] == 0x32))
     {
      // Start continuous measurement
      m_m_active = true;
      m_status_word &= 0x0fff;
      if (wbuffer[0] == 0x08)      m_status_word |= 0x1000;
      else if (wbuffer[0] == 0x32) m_status_word |= 0x6000;
      msg << " Start measurement";
      ret = 0;
     }
    else
     {
      // Unknown command
      msg << "UNKNOWN (" << std::hex << std::showbase << static_cast<int>(cmd)
          << "," << static_cast<int>(wbuffer[0])
          << ") command received." << std::dec << std::noshowbase;
      ret = I2C_DEVICE_SIMUL_UNKNOWN_CMD;
     }

    msg << " [" << std::hex << cmd << "][" << static_cast<int>(wbuffer[0]) << "].";
   }
  else if ((cmd == 0x3f) && (wlength >= 1) && (wbuffer[0] == 0xf9))
   {
    // stop continuous measurement
    m_status_word &= 0x0fff;
    m_m_active = false;
    msg << " Stop measurement [" << std::hex << cmd << "]["
        << static_cast<int>(wbuffer[0]) << "].";
   }
  else if ((cmd == 0xe1) && (wlength >= 1) && (wbuffer[0] == 0x02))
   {
    msg << " Read product identifier";
    if (m_m_active)
     {
      ret = I2C_DEVICE_BUSY;
      msg << " received while device is busy reading out.";
     }
    else
     {
      ret = 0;
      return_word r(static_cast<uint16_t>((m_product_number&0xffff0000)>>16));
      if (rlength >= 3) { r.fill_data_crc(&(rbuffer[0])); ret += 3; }
      else m_retc.push_back(r);
      r = (static_cast<uint16_t>(m_product_number&0xffff));
      if (rlength >= 6) { r.fill_data_crc(&(rbuffer[3])); ret += 3; }
      else m_retc.push_back(r);
      r = (static_cast<uint16_t>((m_serial_number&0xffff000000000000UL)>>48));
      if (rlength >= 6) { r.fill_data_crc(&(rbuffer[6])); ret += 3; }
      else m_retc.push_back(r);
      r = (static_cast<uint16_t>((m_serial_number&0xffff00000000UL)>>32));
      if (rlength >= 9) { r.fill_data_crc(&(rbuffer[9])); ret += 3; }
      else m_retc.push_back(r);
      r = (static_cast<uint16_t>((m_serial_number&0xffff0000)>>16));
      if (rlength >= 12) { r.fill_data_crc(&(rbuffer[12])); ret += 3; }
      else m_retc.push_back(r);
      r = (static_cast<uint16_t>(m_serial_number&0xffff));
      if (rlength >= 15) { r.fill_data_crc(&(rbuffer[15])); ret += 3; }
      else m_retc.push_back(r);
     }
   }
  else if ((cmd == 0xe1) && (wlength >= 1) && (wbuffer[0] == 0x7d))
   {
    msg << " Update concentration - NOP";
    ret = 0;
   }
  else if ((cmd == 0x00) && (wlength >= 1) && (wbuffer[0] == 0x06))
   {
    // soft reset
    m_init();
    msg << " Soft reset.";
   }
  else
   {
    // Unknown command
    msg << "UNKNOWN (" << std::hex << std::showbase << static_cast<int>(cmd);
    if (wlength > 0) msg << "," << static_cast<int>(wbuffer[0]);
    msg << ") command received." << std::dec << std::noshowbase;
    ret = I2C_DEVICE_SIMUL_UNKNOWN_CMD;
   }

  m_dbg.DbgPrint(DBG_CODE, DBG_VALUE, msg.str().c_str());
  return ret;
};

bool
mvm_fw_unit_test_SENSIRION_SFM3019::m_update_measurement()
{
  if (m_last_mtick >= FW_TEST_tick) return true;

  int ret = true;
  m_freading = FW_TEST_pflow.in_f_value(FW_TEST_main_config.get_scaled_ms());
  if (std::isnan(m_freading))
   {
    m_flow_val = 0x7fff;
    ret = false;
   }
  else
   {
    double cread = m_freading * m_scale_factor;
    cread       += m_offset;
    m_flow_val = static_cast<int16_t>(cread);
   }

  m_treading = FW_TEST_qtl_double.value(m_name + "_temperature",FW_TEST_ms);
  if (std::isnan(m_treading))
   {
    m_treading = FW_TEST_qtl_double.value("env_temperature",FW_TEST_ms);
   }

  if (!std::isnan(m_treading))
   {
    m_temp_val = static_cast<uint16_t>(m_treading * 200.);
   }
  else
   {
    m_temp_val = 0xffff;
    ret = false;
   }
  m_last_mtick = FW_TEST_tick;
  return ret;
}

int
mvm_fw_unit_test_TI_ADS1115::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  int ret = -1;

  timespec now;
  ::clock_gettime(CLOCK_REALTIME, &now);
  std::ostringstream msg;
  msg << I2C_DEVICE_module_name << " - ADS1115 - " << m_name << " - "
      << now.tv_sec << ":" << now.tv_nsec/1000000
      << " - ms (scaled):" << FW_TEST_main_config.get_scaled_ms()
      << " - tick:" << FW_TEST_tick << " - ";

  if ((cmd >= 0) && (cmd <= 3))
   {
    if (wlength >= 2)
     {
      m_reg[cmd] = ((wbuffer[0] << 8) | wbuffer[1]);
      if (cmd == CONFIG_REG)
       {
        m_reconfig_gain();
        ret = 0;
       }
     }

    if (rlength >= 2)
     {
      if ((m_reg[CONFIG_REG] & 0x8000) || // One-shot conversion ?
          (m_reg[CONFIG_REG] & 0x0100))   // Continuous conversion ?
       {
        m_reg[CONVERSION_REG] = 0;
        if (m_cur_mux == 4)
         {
          m_o2_concentration = FW_TEST_qtl_double.value("o2_concentration",FW_TEST_ms);
          m_reg[CONVERSION_REG] = (m_o2_concentration - m_o2_sensor_calib_m)/
                                  m_o2_sensor_calib_q;
          msg << "O2 concentration: " << m_o2_concentration << "%";
         }
        else if ((m_cur_mux >= 5) && (m_cur_mux <= 7))
         {
          m_voltage_ref = FW_TEST_qtl_double.value("voltage_ref",FW_TEST_ms);
          uint16_t vrefs;
          if (m_voltage_ref <= 0) vrefs = 0;
          else if (m_voltage_ref > m_vmax) vrefs = 0xffff;
          else vrefs = 0xffff*(m_voltage_ref/m_vmax);
          msg << " Voltage reference: " << m_voltage_ref << " V";

          if (m_cur_mux == 6)
           {
            m_voltage_12v = FW_TEST_qtl_double.value("voltage_12v",FW_TEST_ms);
            msg << " 12V voltage: " << m_voltage_12v << " V";
            m_reg[CONVERSION_REG] = (m_voltage_12v/(2.5*5.)) * vrefs;
           }
          else if (m_cur_mux == 7)
           {
            m_voltage_5v = FW_TEST_qtl_double.value("voltage_5v",FW_TEST_ms);
            msg << " 5V voltage: " << m_voltage_5v << " V";
            m_reg[CONVERSION_REG] = (m_voltage_5v/(2.5*2.)) * vrefs;
           }
          else m_reg[CONVERSION_REG] = vrefs;
         }

        msg << " == [" << std::hex << std::setfill('0')
            << m_reg[CONVERSION_REG] << "]."
            << std::dec << std::noshowbase;
        if (m_reg[CONFIG_REG] & 0x8000) m_reg[CONFIG_REG] &= 0x7fff;
       }

      rbuffer[0] = (m_reg[cmd]&0xff00) >> 8;
      rbuffer[1] = (m_reg[cmd]&0xff);
      msg << " Read register " << std::hex
          << std::showbase << static_cast<int>(cmd)
          << " returning [" << std::hex << std::setfill('0') << std::noshowbase
          << static_cast<int>(rbuffer[0]) << "]["
          << static_cast<int>(rbuffer[1]) << "]." << std::dec;
      ret = 2;
     }
   }
  else
   {
    msg << "UNKNOWN (" << std::hex << std::showbase << static_cast<int>(cmd)
        << ") command received." << std::dec << std::noshowbase;
    ret = I2C_DEVICE_SIMUL_UNKNOWN_CMD;
   }
  m_dbg.DbgPrint(DBG_CODE, DBG_VALUE, msg.str().c_str());
  return ret;
};

void
mvm_fw_unit_test_Supervisor::m_update()
{
  if ( FW_TEST_ms <= m_last_update_ms ) return;

  m_temp = FW_TEST_qtl_double.value("supervisor_temperature",FW_TEST_ms);
  if (std::isnan(m_temp))
   {
    m_temp = FW_TEST_qtl_double.value("env_temperature",FW_TEST_ms);
   }
  if (std::isnan(m_temp))
   {
    m_temp = 0;
   }

  double cval;
  cval = FW_TEST_qtl_double.value("supervisor_pin",FW_TEST_ms);
  if (std::isnan(cval))
   {
    cval = FW_TEST_qtl_double.value("input_line_pressure",FW_TEST_ms);
   }
  if (!std::isnan(cval))
   {
    m_pin = static_cast<float>(cval);
   }
  else m_pin = 0.;

  cval = FW_TEST_qtl_double.value("supervisor_alarms",FW_TEST_ms);
  if (!std::isnan(cval))
   {
    m_alarmsflags = static_cast<uint16_t>(cval);
   }
  else m_alarmsflags = 0;

  cval = FW_TEST_qtl_double.value("wall_power",FW_TEST_ms);
  if (!std::isnan(cval))
   {
    if (cval != 0) m_pwall = true;
    else           m_pwall = false;
   }
  else m_pwall = true;

  if (m_pwall)
   {
    cval = FW_TEST_qtl_double.value("charge_current",FW_TEST_ms);
    m_charge += cval * 0.001 * (FW_TEST_ms - m_last_update_ms);
   }
  else
   {
    cval = FW_TEST_qtl_double.value("discharge_current",FW_TEST_ms);
    m_charge += cval * 0.001 * (FW_TEST_ms - m_last_update_ms);
   }
  if (m_charge < 0.) m_charge = 0.;
  if (m_charge > 100.) m_charge = 100.;

  m_last_update_ms = FW_TEST_ms;
}

int
mvm_fw_unit_test_Supervisor::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  int ret = -1;

  verbose_level vlev = DBG_VALUE;
  timespec now;
  ::clock_gettime(CLOCK_REALTIME, &now);
  std::ostringstream msg;
  msg << I2C_DEVICE_module_name << " - SUPER - "
    << now.tv_sec << ":" << now.tv_nsec/1000000
    << " - ms (scaled):" << FW_TEST_main_config.get_scaled_ms()
    << " - tick:" << FW_TEST_tick << " - command: "
    << std::hex << std::showbase << static_cast<int>(cmd);

  uint16_t val;

  switch (cmd)
   {
    case 0x0:
      FW_TEST_last_watchdog_reset = FW_TEST_tick;
      vlev = DBG_INFO;
      msg << " - watchdog reset";
      ret = 0;
      break;
    // Supervisor response is interpreted as little-endian.
    case 0x50:
      m_update();
      if (rlength < 2) ret = I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
      else
       {
        val = static_cast<uint16_t>(m_pin);
        rbuffer[1] = ((val&0xff00)>>8); rbuffer[0] = (val&0xff);
        ret = 2;
       }
      break;
    case 0x51:
      m_update();
      if (rlength < 2) ret = I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
      else
       {
        val = static_cast<uint16_t>(m_charge);
        rbuffer[1] = ((val&0xff00)>>8); rbuffer[0] = (val&0xff);
        ret = 2;
       }
      break;
    case 0x52:
      m_update();
      if (rlength < 2) ret = I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
      else
       {
        val = (m_pwall ? 0 : 1); // The logic is reversed in the HW module
        rbuffer[1] = ((val&0xff00)>>8); rbuffer[0] = (val&0xff);
        ret = 2;
       }
      break;
    case 0x56:
      m_update();
      if (rlength < 2) ret = I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
      else
       {
        val = static_cast<uint16_t>(m_temp*10);
        rbuffer[1] = ((val&0xff00)>>8); rbuffer[0] = (val&0xff);
        ret = 2;
       }
      break;
    case 0x57:
      m_update();
      if (rlength < 2) ret = I2C_DEVICE_INSUFFICIENT_READ_BUFFER;
      else
       {
        rbuffer[1] = ((m_alarmsflags&0xff00)>>8);
        rbuffer[0] = (m_alarmsflags&0xff);
        ret = 2;
       }
      break;

    default:
      msg << "UNKNOWN command.";
      ret = I2C_DEVICE_SIMUL_UNKNOWN_CMD;
   }

  m_dbg.DbgPrint(DBG_CODE, vlev, msg.str().c_str());
  return ret;
}
