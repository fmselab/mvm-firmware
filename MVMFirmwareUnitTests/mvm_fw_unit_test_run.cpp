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
#include <sstream>
#include <string>

#include <ctime> // nanosleep
#include <cstdio> // serial TTY file access

#include "simulated_fw_board_v4.h"
#include "MVMCore.h"

#include "mvm_fw_unit_test_config.h"
mvm_fw_unit_test_config    FW_TEST_main_config;
quantity_timelines<double> FW_TEST_qtl_double;
qtl_tick_t                 FW_TEST_tick;

// The following can go away when all leftover references disappear from
// the firmware code.
extern WireImpl Wire;
WireImpl Wire;

extern SerialImpl Serial;
SerialImpl Serial;

int
send_command_to_mvm(String sline, MVMCore &mvm)
{
  std::istringstream ils(sline.c_str());

  std::string command;
  std::string param;
  std::string value;
  ils >> command;
  if (command == "get")
   {
    ils >> param;
    if (param.length() > 0)
     {
      String strParam(param.c_str());
      mvm.WriteUART0(mvm.GetParameter(strParam));
      return 0;
     }
   } 
  else if (command == "set")
   {
    ils >> param;
    if (param.length() > 0)
     {
      ils >> value;
      if (value.length() > 0)
       {
        String strParam(param.c_str());
        String strValue(value.c_str());
        if (mvm.SetParameter(strParam, strValue))
         {
          mvm.WriteUART0("valore=OK");
          return 0;
         }
        else mvm.WriteUART0("valore=ERROR:Invalid Command Argument");
       }
     }
   }
  return -1;
}

int 
main (int argc, char *argv[]) 
{
  std::ostringstream usage_string;
  usage_string << "Usage: " << argv[0] << " [-d debug_level]  <JSON config>"
               << std::endl;
  if (argc < 2)
   {
    std::cerr << usage_string.str() << std::endl;
    return 1;
   }
  if (std::string(argv[1]) == "-d")
   {
    if (argc < 4)
     {
      std::cerr << usage_string.str() << std::endl;
     }
    std::istringstream dis(argv[2]);
    dis >> FW_TEST_debug_level;
   } 
  const char *json_conf = argv[argc - 1];

  if (!FW_TEST_main_config.load_config(json_conf))
   {
    std::cerr << argv[0] << ":" 
              <<  FW_TEST_main_config.get_error_string()
              << std::endl;
    return 2;
   }

  FW_TEST_qtl_double.initialize(FW_TEST_main_config.get_conf());
  
  std::string log_file;
  std::ofstream logf;
  if (FW_TEST_main_config.get_string(MVM_FM_confattr_LogFile, log_file))
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
  if (!FW_TEST_main_config.get_string(MVM_FM_confattr_SerialTTY, serial_tty))
   {
    std::cerr << argv[0] << ": Error. Could not find any  " 
              << MVM_FM_confattr_SerialTTY
              << " attribute in " << json_conf << "." << std::endl;
    return 4;
   }

  FILE *ttys = ::fopen(serial_tty.c_str(), "w+");
  if (ttys == 0)
   {
    std::cerr << argv[0] << ": Error. Could not open TTY file " 
              << serial_tty
              << ": " << system_error()
              << "." << std::endl;
    return 5;
   }

  // Would probably need to set up the TTY here. We go via named
  // pipes for the time being.

  Serial.set_ttys(ttys);
  FW_TEST_main_config.get_number<int>(MVM_FM_confattr_SerialPollTimeout,
                                      FW_TEST_serial_poll_timeout);

  qtl_tick_t start_tick, end_tick;
  if (!FW_TEST_main_config.get_number<qtl_tick_t>(MVM_FM_confattr_StartTick,
                                                  start_tick))
   {
    std::cerr << argv[0] << ": Warning. Could not find any  " 
              << MVM_FM_confattr_StartTick
              << " attribute in " << json_conf 
              << ". Using default value: " << start_tick << "." << std::endl;
   }

  if (!FW_TEST_main_config.get_number<qtl_tick_t>(MVM_FM_confattr_EndTick,
                                                  end_tick))
   {
    std::cerr << argv[0] << ": Warning. Could not find any  " 
              << MVM_FM_confattr_EndTick
              << " attribute in " << json_conf 
              << ". Using default value: " << end_tick << "." << std::endl;
   }

  MVMCore the_mvm;
  the_mvm.Init(); // Should check for errors - where ?

  // Main ticker loop 
  for (FW_TEST_tick = start_tick; FW_TEST_tick <= end_tick ; ++FW_TEST_tick)
   {
    the_mvm.Tick();
    if (Serial.available())
     {
      String line = Serial.readStringUntil('\n');
      if (line.length() > 0)
       {
        send_command_to_mvm(line, the_mvm);
       }
     }
    else
     {
      timespec wait = {0, 100000};
      ::nanosleep(&wait, NULL);
     }
   }

  return 0;
}

