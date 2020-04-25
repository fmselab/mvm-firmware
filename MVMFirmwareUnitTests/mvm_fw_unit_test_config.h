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
        TNUM ret;
        const rapidjson::Value& v(m_conf[cname]);
        if (!v.IsNumber()) return false;
        value = v.Get<TNUM>();
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

extern mvm_fw_unit_test_config FW_TEST_main_config;

extern quantity_timelines<double> FW_TEST_qtl_double;
extern qtl_tick_t                 FW_TEST_tick;

#endif /* defined _MVM_FW_TEST_CONFIG_H */
