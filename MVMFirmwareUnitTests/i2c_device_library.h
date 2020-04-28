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
     simulated_i2c_device(name, dbg), m_model(model) { m_init_prom(); }
    mvm_fw_unit_test_TE_MS5525DSO(TE_MS5525DSO_models model,
                            const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg), m_model(model) { m_init_prom(); }
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
};

class
mvm_fw_unit_test_SENSIRION_SFM3019: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_SENSIRION_SFM3019(const std::string &name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg) { m_init(); }
    mvm_fw_unit_test_SENSIRION_SFM3019(const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg) { m_init(); }
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

    void m_init()
     {
      m_m_active = false;
      m_flow_unit = 0x0148; // Standard liters @20C per minute
      m_last_mtick = 0;
      m_status_word = 0x3ff;
      m_scale_factor = 170;
      m_offset = -24576;
     }

    bool m_update_measurement();

    uint8_t m_crc(uint16_t data)
     {
      return m_crc(&data, sizeof(data));
     }

    uint8_t m_crc(int16_t data)
     {
      return m_crc(reinterpret_cast<uint16_t *>(&data), sizeof(data));
     }

    uint8_t m_crc(uint16_t *data, int byte_count)
     {
      return m_crc(reinterpret_cast<uint8_t *>(data), byte_count);
     }

    uint8_t m_crc(uint8_t* data, int count)
     {
      uint16_t cb;
      uint8_t crc = 0xFF;
      uint8_t crc_bit;

      for (cb = 0; cb < count; ++cb)
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

class
mvm_fw_unit_test_TI_ADS1115: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_TI_ADS1115(const std::string &name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg) {}
    mvm_fw_unit_test_TI_ADS1115(const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg) {}
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
    uint16_t m_reg[4];
    int m_cur_mux, m_cur_gain;
    double m_o2_sensor_calib_q, m_o2_sensor_calib_m;
    double m_vmax;
    double m_o2_concentration, m_voltage_ref, m_voltage_12v, m_voltage_5v;
};

struct
TCA_I2C_Multiplexer_command_handler
{
  TCA_I2C_Multiplexer_command_handler(int cmd): m_cmd(cmd) {}
  ~TCA_I2C_Multiplexer_command_handler() {}

  int operator()(uint8_t* a1, int a2, uint8_t* a3, int a4)
   {
    timespec now;
    ::clock_gettime(CLOCK_REALTIME, &now);
    std::ostringstream msg;
    msg << I2C_DEVICE_module_name << " - TCA MUX - " 
      << now.tv_sec << ":" << now.tv_nsec/1000000 << " - tick:"
      << FW_TEST_tick << " - called (NOP) with cmd " << m_cmd;

    return 0;
   }

  private:
   int m_cmd;
};

class
TCA_I2C_supervisor_command_handler
{
  int operator()(uint8_t* a1, int a2, uint8_t* a3, int a4)
   {
    return -1;
   }
};

#endif /* defined _MVM_FW_TEST_I2C_DEVLIB */
