//
// File: simulated_i2c_device.h
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 23-Apr-2020 Initial version.
//
// Description:
// Base class to describe simulated I2C devices.
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
const int I2C_DEVICE_SIMUL_DEAD=-4;
const int I2C_DEVICE_INSUFFICIENT_READ_BUFFER=-5;
const int I2C_DEVICE_NOT_ACTIVE=-6;
const int I2C_DEVICE_NOT_ENABLED=-7;
const int I2C_DEVICE_BUSY=-8;

const std::string I2C_DEVICE_module_name("I2C SIMULATION");

#include "mvm_fw_unit_test_config.h"
#include "DebugIface.h"

struct sim_i2c_devaddr
{
  sim_i2c_devaddr() {}
  sim_i2c_devaddr(uint8_t i_address, int8_t i_muxport): address(i_address), muxport(i_muxport) {}
  uint8_t address;
  int8_t muxport; //-1 indicates ANY
  bool operator< (const sim_i2c_devaddr &other) const
   {
    if (this->muxport == other.muxport) return (this->address < other.address);
    else return (this->muxport < other.muxport);
   }
};

class simulated_i2c_device
{
  public:
    typedef std::function<int (uint8_t* a1, int a2, uint8_t* a3, int a4)> simulated_i2c_cmd_handler_t;
    typedef std::map<uint8_t, simulated_i2c_cmd_handler_t> simulated_i2c_cmd_handler_container_t;

    simulated_i2c_device(const std::string &name, DebugIfaceClass &dbg, const std::string &modname) :
      m_dbg(dbg), m_name(name), m_modname(modname),  m_alive_attr(name)
      { m_alive_attr += "_enable"; }
    simulated_i2c_device(const std::string &name, DebugIfaceClass &dbg, const char *modname) :
      m_dbg(dbg), m_name(name), m_modname(modname),  m_alive_attr(name)
      { m_alive_attr += "_enable"; }
    simulated_i2c_device(const char *name, DebugIfaceClass &dbg, const char *modname) :
      m_dbg(dbg), m_name(name), m_modname(modname), m_alive_attr(name)
      { m_alive_attr += "_enable";}
    virtual ~simulated_i2c_device() {}

    void set_alive_attr(const std::string &attr) { m_alive_attr = attr; }
    // The device name will be used as a prefix for retrieving timeline
    // attributes unless the following is set:
    void set_timeline_prefix(const std::string &prefix)
     {
      m_timeline_prefix = prefix;
     }

    int exchange_message(uint8_t* wbuffer, int wlength,
                         uint8_t *rbuffer, int rlength, bool stop)
     {
      std::ostringstream err;
      err << m_log_lineh();

      if (!alive())
       {
        m_dbg.DbgPrint(DBG_CODE, DBG_INFO, err.str().c_str());
        return I2C_DEVICE_SIMUL_DEAD;
       }
      if ((wbuffer != NULL) && (wlength < 1))
       {
        err << " - missing command.";
        m_dbg.DbgPrint(DBG_CODE, DBG_INFO, err.str().c_str());
        return I2C_DEVICE_SIMUL_NO_CMD;
       }
      uint8_t cmd = 0;
      if (wbuffer != NULL)
       {
        cmd = wbuffer[0];
        ++wbuffer;
       }
      return handle_command(cmd, wbuffer, wlength-1, rbuffer, rlength);
     }

    void add_command_handler(uint8_t cmd, simulated_i2c_cmd_handler_t hnd)
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

    virtual int handle_command(uint8_t cmd, uint8_t *wbuffer, int wlength,
                                            uint8_t *rbuffer, int rlength)
     {
      simulated_i2c_cmd_handler_container_t::iterator cmdp;
      cmdp = m_cmd_handlers.find(cmd);
      if (cmdp != m_cmd_handlers.end())
       {
        return cmdp->second(wbuffer, wlength, rbuffer, rlength);
       }
      else
       {
        std::ostringstream err;
        err << m_log_lineh()
            << "No handler for command "
            << std::hex << std::showbase << static_cast<int>(cmd)
            << "." << std::dec << std::noshowbase;
        m_dbg.DbgPrint(DBG_CODE, DBG_INFO, err.str().c_str());
        return I2C_DEVICE_SIMUL_UNKNOWN_CMD;
       }
     }


  const std::string &get_name() const { return m_name; }
  DebugIfaceClass &get_dbg() const { return m_dbg; }

  bool alive()
   {
    if (m_alive_attr.length() <= 0) return true;
    double dalive = (FW_TEST_qtl_double.value(m_alive_attr,
                                 FW_TEST_main_config.get_scaled_ms()));
    if ((std::isnan(dalive)) || (dalive != 0)) return true;
    return false;
   }

  protected:

    std::string m_log_lineh(qtl_ms_t now_ms = -1) const
     {
      if (now_ms < 0) now_ms = FW_TEST_ms;
      timespec now;
      ::clock_gettime(CLOCK_REALTIME, &now);
      std::ostringstream msg;
      msg << I2C_DEVICE_module_name 
          << " - " << m_modname
          << " - " << m_name << " - "
          << now.tv_sec << ":" << now.tv_nsec/1000000 
          << " - ms (scaled):" << now_ms
          << " - tick:" << FW_TEST_tick << " - ";
      return msg.str();
     }

    DebugIfaceClass &m_dbg;
    simulated_i2c_cmd_handler_container_t m_cmd_handlers; 
    std::string m_name;
    std::string m_modname;
    std::string m_alive_attr;
    std::string m_timeline_prefix;
};

typedef std::map<sim_i2c_devaddr, simulated_i2c_device *> simulated_i2c_devices_t;

#endif /* defined _I2C_DEVICE_SIMUL_H */
