//
// File: simulate_i2c_devices.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 24-Apr-2020 Initial version.
//
// Description:
// Knowledge to populate and implement the MVM I2C device zoo.
//

#include "simulate_i2c_devices.h"
#include "i2c_device_library.h"

void
simulate_i2c_devices::init_hw(const test_hardware_t &hwl)
{
  test_hardware_t::const_iterator it;
  test_hardware_t::const_iterator ite = hwl.end();

  for (it = hwl.begin(); it != ite; ++it)
   {
    simulated_i2c_device *nd;

    if (it->second.first == TEST_TE_MS5525DSO)
     {
      if (it->second.second == "PI3")
        nd = new mvm_fw_unit_test_TE_MS5525DSO(pp005GS, it->second.second, DebugIface);
      else
        nd = new mvm_fw_unit_test_TE_MS5525DSO(pp001DS, it->second.second, DebugIface); 
     }
    else if (it->second.first == TEST_SENSIRION_SFM3019)
     {
      nd = new mvm_fw_unit_test_SENSIRION_SFM3019(it->second.second, DebugIface); 
     }
    else if (it->second.first == TEST_TI_ADS1115)
     {
      nd = new mvm_fw_unit_test_TI_ADS1115(it->second.second, DebugIface); 
     }
    else if (it->second.first == TEST_TCA_I2C_MULTIPLEXER)
     {
      nd = new mvm_fw_unit_test_Multiplexer(it->second.second, DebugIface, m_cur_mux);
     }
    else if (it->second.first == TEST_XXX_SUPERVISOR)
     {
      nd = new mvm_fw_unit_test_Supervisor(it->second.second, DebugIface); 
     }
    if (nd) add_device(it->first.address, it->first.muxport, nd);
   }
}
