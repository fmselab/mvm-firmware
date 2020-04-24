//
// File: mvm_fw_unit_test_run.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 23-Apr-2020 Initial version.
//
// Description:
// Main program to run the MVM Firmware unit tests.
//

#include "quantity_timelines.hpp"

#include <iostream>
#include <fstream>

#include <ctime> // nanosleep

#include "generic_definitions.h"
#undef  HARDWARE_TARGET_PLATFORM_V3
#undef  HARDWARE_TARGET_PLATFORM_V4

#include "simulated_fw_board_v4.h" // Our HW abstraction
#include "MVMCore.h"

// The following can go away when all leftover references disappear from
// the firmware code.
extern WireImpl Wire;
WireImpl Wire;

std::fstream IOS_ttys;
extern SerialImpl Serial;
SerialImpl Serial(IOS_ttys);

// Real quick - ugh. This should eventually move to some config module
typedef rapidjson::Document mvm_fw_test_config_t;
static bool get_config_string(const mvm_fw_test_config_t &conf,
                              const std::string &name,
                              std::string &value)
{
  const char *cname=name.c_str();
  if (conf.HasMember(cname))
   {
    const rapidjson::Value& v(conf[cname]);
    if (!v.IsString()) return false;
    value = v.GetString();
    return true;
   }
  return false;
}

#include <cerrno>
#include <cstring> // strerror()

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

std::ostream &operator<< (std::ostream &os, const system_error &serr)
{
  return (os << serr.get_err());
}

int 
main (int argc, char *argv[]) 
{
  std::string expr;
  if (argc < 2)
   {
    std::cerr << "Usage: " << argv[0] << " <JSON config>"
              << std::endl;
    return 1;
   }
  const char *json_conf = argv[1];

  mvm_fw_test_config_t main_config;

   {
    std::ifstream ifs(json_conf);
    if (!ifs.good())
     {
      std::cerr << argv[0] << ": Error reading from configuration file " 
                << json_conf 
                << ": " << system_error()
                << "." << std::endl;
      return 2;
     }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::ParseResult pres = main_config.ParseStream(isw);
    if (!pres)
     {
      std::cerr << argv[0] << ": Error parsing configuration file " 
                << json_conf << ": "
                << rapidjson::GetParseError_En(pres.Code())
                << "." << std::endl;
      return 3;
     }
   }

  // These should be eventually move elsewhere, too.
  const std::string MVM_FM_confattr_LogFile("LogFile");
  const std::string MVM_FM_confattr_SerialTTY("SerialTTY");

  std::string log_file;
  std::ofstream logf;
  if (get_config_string(main_config, MVM_FM_confattr_LogFile, log_file))
   {
    logf.open(log_file, std::ofstream::out | std::ofstream::app);
    if (!logf.good())
     {
      std::cerr << argv[0] << ": Warning. Could not write to log file " 
                << log_file
                << ": " << system_error()
                << "." << std::endl;
     }
   }
  else
   {
    std::cerr << argv[0] << ": Warning. Could not find any  " 
              << MVM_FM_confattr_LogFile
              << " attribute in " << json_conf << "." << std::endl;
   }

  std::string serial_tty; 
  if (!get_config_string(main_config, MVM_FM_confattr_SerialTTY, serial_tty))
   {
    std::cerr << argv[0] << ": Error. Could not find any  " 
              << MVM_FM_confattr_SerialTTY
              << " attribute in " << json_conf << "." << std::endl;
    return 4;
   }

  IOS_ttys.open(serial_tty, std::ofstream::out | std::ofstream::in);
  if (!IOS_ttys.good())
   {
    std::cerr << argv[0] << ": Error. Could not open TTY file " 
              << serial_tty
              << ": " << system_error()
              << "." << std::endl;
    return 5;
   }

  // Would probably need to set up the TTY here. We go via named
  // pipes for the time being.

  MVMCore the_mvm;
  the_mvm.Init(); // Should check for errors - where ?

  // Main ticker loop 
  uint64_t curtick;
  for (curtick = 0; ;++curtick)
   {
    the_mvm.Tick();
    timespec wait = {0, 100000};
    ::nanosleep(&wait, NULL);
   }

  return 0;
}

