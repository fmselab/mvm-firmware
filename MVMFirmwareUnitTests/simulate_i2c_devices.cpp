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

#include "mvm_fw_unit_test_config.h"
#include "simulated_i2c_device.h"

#include <stdint.h>

class
simulate_i2c_devices
{
  public:
    simulate_i2c_devices(): m_conf(FW_TEST_main_config) { m_init(); }
    simulate_i2c_devices(mvm_fw_unit_test_config &conf): m_conf(conf) { m_init();}
    ~simulate_i2c_devices();

    int
    exchange_message(uint8_t address, int8_t muxport,
          uint8_t* wbuffer, int wlength, uint8_t *rbuffer, bool stop)
     {
      simulated_i2c_device_address addr(address, muxport);
      simulated_i2c_devices_t::iterator dit;
      dit = m_devs.find(addr);
      if (dit != m_devs.end())
       {
        return dit->second.exchange_message(wbuffer, wlength, rbuffer, stop);
       }
      else return I2C_DEVICE_SIMUL_NOT_FOUND;
     }

    bool
    add_device(uint8_t address, int8_t muxport,
               simulated_i2c_device &dev)
     {
      simulated_i2c_device_address addr(address, muxport);
      simulated_i2c_devices_t::const_iterator dit;
      dit = m_devs.find(addr);
      if (dit == m_devs.end())
       {
        m_devs.insert(std::make_pair(addr, dev));
        return true;
       }
      return false;
     }

  private:
    void m_init();
    mvm_fw_unit_test_config &m_conf;
    simulated_i2c_devices_t m_devs;
};

void
simulate_i2c_devices::m_init()
{
  // XXX still to do.
}
