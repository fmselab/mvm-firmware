//
// File: simulated_i2c_device.h
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 23-Apr-2020 Initial version.
//
// Description:
// Base class to describe simulated I2C devices
//

#ifndef _I2C_DEVICE_SIMUL_H
#define _I2C_DEVICE_SIMUL_H

#include <functional>
#include <sstream>
#include <string>
#include <map>
#include <stdint.h> // <cstdint> is C++11.

const int I2C_DEVICE_SIMUL_NOT_FOUND=-1;
const int I2C_DEVICE_SIMUL_NO_CMD=-2;
const int I2C_DEVICE_SIMUL_UNKNOWN_CMD=-3;
const std::string I2C_DEVICE_module_name("I2C SIMULATION");

#include "DebugIface.h"

struct simulated_i2c_device_address
{
  simulated_i2c_device_address() {}
  simulated_i2c_device_address(uint8_t i_address, int8_t i_muxport): address(i_address), muxport(i_muxport) {}
  uint8_t address;
  int8_t muxport; //-1 indicates ANY
  bool operator< (const simulated_i2c_device_address &other) const
   {
    if (muxport < other.muxport) return true;
    if (address < other.address) return true;
    return false;
   }
};

class simulated_i2c_device
{
  public:
    typedef std::function<int (uint8_t* a1, int a2, uint8_t* a3)> simulated_i2c_cmd_handler_t;
    typedef std::map<uint8_t, simulated_i2c_cmd_handler_t> simulated_i2c_cmd_handler_container_t;

    simulated_i2c_device(const std::string &name, DebugIfaceClass &dbg); 
    simulated_i2c_device(const char *name, DebugIfaceClass &dbg);
    virtual ~simulated_i2c_device() {}

    int exchange_message(uint8_t* wbuffer, int wlength, uint8_t *rbuffer, bool stop)
     {
      if (wlength < 1)
       {
        std::ostringstream err;
        err << I2C_DEVICE_module_name << ": Missing command for device '"
            << m_name << "'.";
        m_dbg.DbgPrint(DBG_CODE, DBG_INFO, err.str().c_str());
        return I2C_DEVICE_SIMUL_NO_CMD;
       }
      uint8_t cmd = wbuffer[0]&0x7f;
      return handle_command(cmd, wbuffer+1, wlength-1, rbuffer);
     }
    void add_command_handler(uint8_t cmd, simulated_i2c_cmd_handler_t &hnd)
     {
      simulated_i2c_cmd_handler_container_t::iterator cmdp;
      cmdp = m_cmd_handlers.find(cmd);
      if (cmdp != m_cmd_handlers.end())
       {
        cmdp->second = hnd;
       }
      else
       {
        m_cmd_handlers.insert(std::make_pair(cmd, hnd));
       }
     }
    virtual int handle_command(uint8_t cmd, uint8_t *wbuffer,
                               int wlength, uint8_t *rbuffer)
     {
      simulated_i2c_cmd_handler_container_t::iterator cmdp;
      cmdp = m_cmd_handlers.find(cmd);
      if (cmdp != m_cmd_handlers.end())
       {
        return cmdp->second(wbuffer, wlength, rbuffer);
       }
      else
       {
        std::ostringstream err;
        err << I2C_DEVICE_module_name << ": No handler in device '"
            << m_name << "' for command"
            << std::hex << std::showbase << cmd << ".";
        m_dbg.DbgPrint(DBG_CODE, DBG_INFO, err.str().c_str());
        return I2C_DEVICE_SIMUL_UNKNOWN_CMD;
       }
     }

  const std::string &get_name() const { return m_name; }

  protected:
    DebugIfaceClass &m_dbg;
    simulated_i2c_cmd_handler_container_t m_cmd_handlers; 

  private:
    std::string m_name;
};

typedef std::map<simulated_i2c_device_address, simulated_i2c_device> simulated_i2c_devices_t;

#endif /* defined _I2C_DEVICE_SIMUL_H */
