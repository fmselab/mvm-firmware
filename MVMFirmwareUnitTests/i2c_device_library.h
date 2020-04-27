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

class
mvm_fw_unit_test_TE_MS5525DSO: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_TE_MS5525DSO(const std::string &name, DebugIfaceClass &dbg):
     simulated_i2c_device(name, dbg) { m_init_prom(); } 
    mvm_fw_unit_test_TE_MS5525DSO(const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg) { m_init_prom(); } 
    ~mvm_fw_unit_test_TE_MS5525DSO() {}

    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength);
    private:
    void m_init_prom();
    uint16_t m_prom[8];
    bool m_want_to_read_pressure;
};

class
mvm_fw_unit_test_SENSIRION_SFM3019: public simulated_i2c_device
{
  public:
    mvm_fw_unit_test_SENSIRION_SFM3019(const std::string &name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg) {} 
    mvm_fw_unit_test_SENSIRION_SFM3019(const char *name, DebugIfaceClass &dbg) :
     simulated_i2c_device(name, dbg) {} 
    ~mvm_fw_unit_test_SENSIRION_SFM3019() {}

    int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                    uint8_t *rbuffer, int rlength);
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
};

struct
TCA_I2C_Multiplexer_command_handler
{
  int operator()(uint8_t* a1, int a2, uint8_t* a3, int a4)
   {
    return -1;
   }
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
