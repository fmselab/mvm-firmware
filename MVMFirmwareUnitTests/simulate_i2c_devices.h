//
// File: simulate_i2c_devices.h
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 25-Apr-2020 Initial version.
//
// Description:
// Knowledge to populate and implement the MVM I2C device zoo.
//

#ifndef _I2C_DEVICE_SIMULATE_H
#define _I2C_DEVICE_SIMULATE_H

#include "mvm_fw_unit_test_config.h"
#include "simulated_i2c_device.h"

#include <stdint.h>

class
simulate_i2c_devices
{
  public:
    simulate_i2c_devices(): m_conf(FW_TEST_main_config) {} 
    simulate_i2c_devices(mvm_fw_unit_test_config &conf): m_conf(conf) {}
    ~simulate_i2c_devices()
     {
      simulated_i2c_devices_t::iterator dit;
      for(dit = m_devs.begin(); dit != m_devs.end(); ++dit)
       {
        if (dit->second) delete (dit->second);
       }
     }

    int
    exchange_message(uint8_t address, int8_t muxport,
                     uint8_t* wbuffer, int wlength,
                     uint8_t *rbuffer, int rlength, bool stop)
     {
      if (m_devs.size() <= 0) return I2C_DEVICE_SIMUL_NOT_FOUND;
      sim_i2c_devaddr addr(address, muxport);
      simulated_i2c_devices_t::iterator dit;
      dit = m_devs.find(addr);
      if (dit != m_devs.end())
       {
        return dit->second->exchange_message(wbuffer, wlength,
                                            rbuffer, rlength, stop);
       }
      else return I2C_DEVICE_SIMUL_NOT_FOUND;
     }

    int
    exchange_message(const sim_i2c_devaddr &dad,
                     uint8_t* wbuffer, int wlength,
                     uint8_t *rbuffer, int rlength, bool stop)
     {
      if (m_devs.size() <= 0) return I2C_DEVICE_SIMUL_NOT_FOUND;
      simulated_i2c_devices_t::iterator dit;
      dit = m_devs.find(dad);
      if (dit != m_devs.end())
       {
        return dit->second->exchange_message(wbuffer, wlength,
                                             rbuffer, rlength, stop);
       }
      else return I2C_DEVICE_SIMUL_NOT_FOUND;
     }

    bool 
    alive (const sim_i2c_devaddr &dad)
     {
      if (m_devs.size() <= 0) return false;
      simulated_i2c_devices_t::iterator dit;
      dit = m_devs.find(dad);
      if (dit != m_devs.end())
       {
        return dit->second->alive();
       }
      return false;
     }

    bool
    add_device(uint8_t address, int8_t muxport,
               simulated_i2c_device *dev)
     {
      if (!dev) return false;
      sim_i2c_devaddr addr(address, muxport);
      simulated_i2c_devices_t::const_iterator dit;
      dit = m_devs.find(addr);
      if (dit == m_devs.end())
       {
        m_devs.insert(std::make_pair(addr, dev));
        return true;
       }
      return false;
     }

    void init_hw(const test_hardware_t &hwl);

  private:
    mvm_fw_unit_test_config &m_conf;
    simulated_i2c_devices_t m_devs;
};
#endif /* defined _I2C_DEVICE_SIMULATE_H */
