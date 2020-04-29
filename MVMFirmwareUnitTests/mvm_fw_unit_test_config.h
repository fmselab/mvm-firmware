//
// File: mvm_fw_unit_test_config.h
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 24-Apr-2020 Initial version.
//
// Description:
// Moved here the methods to access the JSON configuration file,
// with a static instance of the config handle, as this seems so fashionable
// around this project.
//
 
#ifndef _MVM_FW_TEST_CONFIG_H
#define _MVM_FW_TEST_CONFIG_H

#include <string>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <cerrno>
#include <cstring> // strerror()

#include "quantity_timelines.hpp"
struct sim_i2c_devaddr;

class system_error
{
  public:
    // We do this in the C way to keep to C++98 standard.
    system_error(): m_errs(::strerror(errno)) {}
    ~system_error() {}
    const std::string &get_err() const { return m_errs; }

  private:
    std::string m_errs;
};

std::ostream& operator<< (std::ostream &os, const system_error &serr);

const std::string MVM_FM_confattr_LogFile("LogFile");
const std::string MVM_FM_confattr_SerialTTY("SerialTTY");
const std::string MVM_FM_confattr_StartTick("start_tick");
const std::string MVM_FM_confattr_EndTick("end_tick");

class mvm_fw_unit_test_config
{
  public: 

    typedef rapidjson::Document mvm_fw_test_config_t;

    mvm_fw_unit_test_config(): m_valid(false) {}
    mvm_fw_unit_test_config(const std::string &conf_file): m_valid(false)
     {
      load_config(conf_file);
     }
    ~mvm_fw_unit_test_config() {}

    bool get_string(const std::string &name,
                          std::string &value)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& v(m_conf[cname]);
        if (!v.IsString()) return false;
        value = v.GetString();
        return true;
       }
      return false;
     }

    template<typename TNUM>
    bool get_number(const std::string &name, TNUM &value)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& v(m_conf[cname]);
        if (!v.IsNumber()) return false;
        value = v.Get<TNUM>();
        return true;
       }
      return false;
     }

    template<typename TNUM>
    bool get_num_array(const std::string &name, TNUM *value, int size)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& a(m_conf[cname]);
        if (!a.IsArray()) return false;
        for (rapidjson::SizeType i = 0; ((i < a.Size())&&(i < size)); i++)
         {
          const rapidjson::Value& v(a[i]);
          if (!(v.IsNumber()))
           {
            value[i] = 0;
            continue;
           }
          value[i] = v.Get<TNUM>();
         }
        return true;
       }
      return false;
     }

    bool get_ushort_array(const std::string &name, uint16_t *value, int size)
     {
      // It seems that  no template classes can be instantiated for
      // uint16_t's.
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& a(m_conf[cname]);
        if (!a.IsArray()) return false;
        for (rapidjson::SizeType i = 0; ((i < a.Size())&&(i < size)); i++)
         {
          const rapidjson::Value& v(a[i]);
          if (!(v.IsNumber()))
           {
            value[i] = 0;
            continue;
           }
          value[i] = static_cast<uint16_t>(v.GetInt());
         }
        return true;
       }
      return false;
     }

    bool get_bool(const std::string &name, bool &value)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& v(m_conf[cname]);
        if (!v.IsBool()) return false;
        value = v.GetBool();
        return true;
       }
      return false;
     }

    bool load_config(const std::string &conf_file);
    const mvm_fw_test_config_t &get_conf() const { return m_conf; }

    const std::string &get_error_string() const { return m_error_string; }

  private: 

    std::string m_conf_file;
    std::string m_error_string;
    bool m_valid;
    mvm_fw_test_config_t m_conf;
};

extern quantity_timelines<double> FW_TEST_qtl_double;
extern qtl_tick_t                 FW_TEST_tick;

class 
mvm_fw_gpio_devs
{
  public:
    enum
    mvm_fw_bool_regs
     {
      BREATHE, 
      OUT_VALVE,
      BUZZER,
      ALARM_LED,
      ALARM_RELAY,
      LAST_REG
     };

    bool set(mvm_fw_bool_regs dev, bool value)
     {
      double enable;
      bool ret = false;
      enable = FW_TEST_qtl_double.value(std::string(m_names[dev])+"_enable",
                                        FW_TEST_tick);
      timespec now;
      ::clock_gettime(CLOCK_REALTIME, &now);
      m_msg.clear();
      m_msg << "GPIO_DEVS" << " - " << m_names[dev] << " - "
      << now.tv_sec << ":" << now.tv_nsec/1000000 << " - tick:"
      << FW_TEST_tick << " - ";

      if (std::isnan(enable) || 
          (!std::isnan(enable) && (enable != 0)))
       {
        m_devs[dev] = value;
        m_msg << "value set to " << value;
        ret = true;
       }
      else
       {
        m_msg << "device disabled in config. NOT set to " << value
              << " - current value is " << m_devs[dev];
       }
      return ret;
     }

    bool set_pv1(uint16_t value)
     {
      double enable;
      bool ret = false;
      enable = FW_TEST_qtl_double.value("PV1_enable", FW_TEST_tick);
      timespec now;
      ::clock_gettime(CLOCK_REALTIME, &now);
      m_msg.clear();
      m_msg << "GPIO_DEVS" << " - PV1 - "
      << now.tv_sec << ":" << now.tv_nsec/1000000 << " - tick:"
      << FW_TEST_tick << " - ";

      if (std::isnan(enable) || 
          (!std::isnan(enable) && (enable != 0)))
       {
        m_pv1_value = value;
        m_msg << "value set to " << value;
        ret = true;
       }
      else
       {
        m_msg << "device disabled in config. NOT set to " << value
              << " - current value is " << m_pv1_value;
       }
      return ret;
     }

    const std::string &get_error_msg()
     {
      m_msg_str = m_msg.str();
      return m_msg_str;
     }

    bool operator[](mvm_fw_bool_regs dev) const { return m_devs[dev]; }

    uint16_t get_pv1() const { return m_pv1_value; }
    double get_pv1_fraction() const
     {
      return (static_cast<double>(m_pv1_value) / 0xffff);
     }

  private:
    bool m_devs[LAST_REG];
    std::ostringstream m_msg;
    std::string m_msg_str;
    uint16_t m_pv1_value;
    const char *m_names[LAST_REG] = { "BREATHE", "OUT_VALVE", "BUZZER",
                                      "ALARM_LED", "ALARM_RELAY" };
};

extern mvm_fw_gpio_devs FW_TEST_gdevs;

class
mvm_fw_unit_test_pflow
{
  public:
    mvm_fw_unit_test_pflow() { m_init(); }
    ~mvm_fw_unit_test_pflow() { m_init(); }

    double p_value(const std::string &name, qtl_tick_t t);
    double f_value(qtl_tick_t t);

    enum p_sensors
     {
      PS0,
      PS1,
      PS2,
      LAST_PS
     };

  private:
    qtl_tick_t m_last_tick;
    void m_init();
    void m_evolve(qtl_tick_t t);
    double m_m_resistance, m_v_resistance;
    double m_ps1_fraction, m_ps2_fraction;
    double m_overpressure;
    double m_capacity;
    double m_volume;
    double m_flow;
    double m_p[LAST_PS];
};

extern mvm_fw_unit_test_pflow FW_TEST_pflow;

/* Hardware map */

enum FW_TEST_devices
{
  TEST_TE_MS5525DSO,
  TEST_SENSIRION_SFM3019,
  TEST_TI_ADS1115,
  TEST_TCA_I2C_MULTIPLEXER,
  TEST_XXX_SUPERVISOR
};

typedef std::map<sim_i2c_devaddr, std::pair<FW_TEST_devices, std::string> > test_hardware_t;
extern test_hardware_t FW_TEST_hardware;
extern qtl_tick_t FW_TEST_last_watchdog_reset;

extern mvm_fw_unit_test_config FW_TEST_main_config;

#endif /* defined _MVM_FW_TEST_CONFIG_H */
