//
// File: i2c_device_library.h
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

#ifndef _MVM_FW_TEST_I2C_DEVLIB
#define _MVM_FW_TEST_I2C_DEVLIB

#include <deque>

#include "DebugIface.h"
#include "simulated_i2c_device.h"
#include "mvm_fw_unit_test_config.h"

enum TE_MS5525DSO_models
{
  pp001DS,
  pp002GS,
  pp002DS,
  pp005GS,
  pp005DS,
  pp015GS,
  pp015AS,
  pp015DS,
  pp030AS,
  pp030GS,
  pp030DS
};

class
mvm_fw_unit_test_TE_MS5525DSO: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_TE_MS5525DSO(TE_MS5525DSO_models model,
                            const std::string &name, DebugIfaceClass &dbg):
     simulated_i2c_device(name, dbg, "MS5525DSO"), m_model(model) { m_init_prom(); }
    mvm_fw_unit_test_TE_MS5525DSO(TE_MS5525DSO_models model,
                            const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "MS5525DSO"), m_model(model) { m_init_prom(); }
    ~mvm_fw_unit_test_TE_MS5525DSO() {}

    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength);
  private:
    void m_init_prom();
    TE_MS5525DSO_models m_model;
    uint16_t m_prom[8];
    int      m_q[6];
    bool m_want_to_read_pressure;
    double m_preading, m_treading;
    bool m_cmd_0_in, got_treading;
};

class
mvm_fw_unit_test_SENSIRION_SFM3019: public simulated_i2c_device
{
  public:


    struct return_word
     {
      return_word(uint16_t w)
       {
        m_w.uw = w;
        m_crcval = m_crc(m_w.uw);
       }
      return_word(int16_t w)
       {
        m_w.sw = w;
        m_crcval = m_crc(m_w.uw);
       }
      ~return_word() {}
       
      return_word &operator= (const uint16_t &w)
       {
        m_w.uw = w;
        m_crcval = m_crc(m_w.uw);
        return *this;
       }

      return_word &operator= (const int16_t &w)
       {
        m_w.sw = w;
        m_crcval = m_crc(m_w.uw);
        return *this;
       }

      void fill_data_crc(uint8_t *ptr)
       {
        ptr[0] = ((m_w.uw & 0xff00) >> 8);
        ptr[1] = (m_w.uw & 0xff);
        ptr[2] = m_crcval;
       }

      private:

        union { uint16_t uw; int16_t sw; } m_w;
        uint8_t m_crcval;
        uint8_t m_crc(uint16_t dw)
         {
          uint16_t cb;
          uint8_t crc = 0xFF;
          uint8_t crc_bit;
          // CRC is big-endian.
          uint8_t data[2];
          data[0] = static_cast<uint8_t>((dw&0xff00)>>8);
          data[1] = static_cast<uint8_t>(dw&0xff);
    
          for (cb = 0; cb < sizeof(data); ++cb)
           {
            crc ^= (data[cb]);
            for (crc_bit = 8; crc_bit > 0; --crc_bit)
             {
              if (crc & 0x80)
               crc = (crc << 1) ^ 0x31; // CRC 'polynomial'
              else
               crc = (crc << 1);
             }
           }
          return crc;
         }
     };

    typedef std::deque<return_word> return_word_container_t; 
 
    mvm_fw_unit_test_SENSIRION_SFM3019(const std::string &name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "SFM3019") { m_init(); }
    mvm_fw_unit_test_SENSIRION_SFM3019(const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "SFM3019") { m_init(); }
    ~mvm_fw_unit_test_SENSIRION_SFM3019() {}

    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength);
  private:

    bool m_m_active;
    qtl_tick_t m_last_mtick;

    uint16_t m_scale_factor, m_flow_unit, m_status_word;
    int16_t m_flow_val, m_temp_val, m_offset;
    const uint32_t m_product_number = 0x04020611;
    const uint64_t m_serial_number  = 2020123456;
    double m_freading, m_treading;

    return_word_container_t m_retc;

    void m_init()
     {
      m_m_active = false;
      m_flow_unit = 0x0148; // Standard liters @20C per minute
      m_last_mtick = 0;
      m_status_word = 0x3ff;
      m_scale_factor = 170;
      m_offset = -24576;
      m_freading = m_treading = 0.;
     }

    bool m_update_measurement();


};


class
mvm_fw_unit_test_Amphenol_NPAx00: public simulated_i2c_device
{
  public:
    enum Amphenol_NPAx00_pressure_ranges
    {
      AMPHENOL_NPAX00_10_INCH_WATER,
      AMPHENOL_NPAX00_1_PSI,
      AMPHENOL_NPAX00_5_PSI,
      AMPHENOL_NPAX00_15_PSI,
      AMPHENOL_NPAX00_30_PSI,
      AMPHENOL_NPAX00_LAST
    };
    double pressure_range[AMPHENOL_NPAX00_LAST] =
     {
       // All ranges expressed in centimeters of water.
       25.4, // 10 Inches of water
       70.30696, // 1 PSI
       351.5348, // 5 PSI
       1054.6044, // 15 PSI
       2109.2088  // 30 PSI
     };

    mvm_fw_unit_test_Amphenol_NPAx00(double pressure_range,
                            const std::string &name, DebugIfaceClass &dbg):
     simulated_i2c_device(name, dbg, "NPAx00"), m_prange(pressure_range) { }
    mvm_fw_unit_test_Amphenol_NPAx00(double pressure_range,
                            const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "NPAx00"), m_prange(pressure_range) { }
    ~mvm_fw_unit_test_Amphenol_NPAx00() {}

    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength);
  private:
    double m_prange;
    double m_preading, m_treading;
};

class
mvm_fw_unit_test_TI_ADS1115: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_TI_ADS1115(const std::string &name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "ADS1115") { m_init(); }
    mvm_fw_unit_test_TI_ADS1115(const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "ADS1115") { m_init(); }
    ~mvm_fw_unit_test_TI_ADS1115() {}

    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength);
    enum registers
     {
      CONVERSION_REG,
      CONFIG_REG,
      LO_THRESHOLD,
      HI_THRESHOLD
     };

  private:
    void m_init()
     {
      m_reg[CONVERSION_REG] = 0;
      m_reg[CONFIG_REG] = 0x8583;
      m_reconfig_gain();
      m_reg[LO_THRESHOLD] = 0x8000;
      m_reg[HI_THRESHOLD] = 0x7fff;
      if (!FW_TEST_main_config.get_number<double>("o2_sensor_calib_q", m_o2_sensor_calib_q))
       {
        m_o2_sensor_calib_q = 0.00452689;
       }
      if (!FW_TEST_main_config.get_number<double>("o2_sensor_calib_m", m_o2_sensor_calib_m))
       {
        m_o2_sensor_calib_m = -1.63;
       }
     }
    void m_reconfig_gain ()
     {
      m_cur_mux = ((m_reg[CONFIG_REG] & 0x7000) >> 12);
      m_cur_gain = ((m_reg[CONFIG_REG] & 0x0e00) >> 9);
      if (m_cur_gain > 5) m_cur_gain = 5;
      m_vmax = (6.144/(1<<m_cur_gain));
     }

    uint16_t m_reg[4];
    int m_cur_mux, m_cur_gain;
    double m_o2_sensor_calib_q, m_o2_sensor_calib_m;
    double m_vmax;
    double m_o2_concentration, m_voltage_ref, m_voltage_12v, m_voltage_5v;
};

class
mvm_fw_unit_test_Supervisor: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_Supervisor(const std::string &name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "SUPER") { m_init(); }
    mvm_fw_unit_test_Supervisor(const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg, "SUPER") { m_init(); }
    ~mvm_fw_unit_test_Supervisor() {}

    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength);
  private:
    void m_init()
     {
      m_last_update_ms = FW_TEST_ms;
      if (!FW_TEST_main_config.get_number<double>("initial_battery_charge", m_charge))
       {
        m_charge = 100.;
       }
     }
    void m_update();

    qtl_ms_t m_last_update_ms;
    bool m_pwall;
    float m_pin;
    double m_temp;
    uint16_t m_alarmsflags;
    double m_charge;
};

class
mvm_fw_unit_test_Multiplexer: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_Multiplexer(const std::string &name, DebugIfaceClass &dbg,
                                 int8_t &muxref):
                                 simulated_i2c_device(name, dbg, "TCAMUX"), m_mux(muxref) {}
    ~mvm_fw_unit_test_Multiplexer() {}
  
    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength)
     {
      std::ostringstream msg;
      msg << m_log_lineh() << "called with cmd " << cmd;
      uint8_t cmux = cmd;
      int8_t mux;
      for (int mux=0; mux<(sizeof(cmux)*8); ++mux)
       {
        if ((cmux&1) != 0) break; 
        cmux >>= 1; 
       }
      if (cmux != 1) // More that one bit set in  command.
       {
        msg << " - malformed command ("  << std::hex << std::showbase
            << cmd << std::dec << std::noshowbase << ")";
       }
      else
       {
        m_mux = mux;
        msg << " - mux set to: " << mux;
       }
      m_dbg.DbgPrint(DBG_CODE, DBG_INFO, msg.str().c_str());
      return 0;
     }

  private:
    int8_t &m_mux;
};

#endif /* defined _MVM_FW_TEST_I2C_DEVLIB */
