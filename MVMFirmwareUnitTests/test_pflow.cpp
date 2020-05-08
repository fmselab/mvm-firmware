//
// File: test_pflow.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
//  8-May-2020 Initial version.
//
// Description:
// Independent test of the pressure/flow model.
//

#include "quantity_timelines.hpp"
#include "mvm_fw_unit_test_config.h"
mvm_fw_unit_test_config    FW_TEST_main_config;
quantity_timelines<double> FW_TEST_qtl_double;
qtl_tick_t                 FW_TEST_tick;
qtl_ms_t                   FW_TEST_ms;
mvm_fw_gpio_devs           FW_TEST_gdevs;


#include <iostream>

#include <ctime>

int 
main (int argc, char *argv[]) 
{
  if (argc < 2)
   {
    std::cerr << "Usage: " << argv[0] << " <JSON config>"
              << std::endl;
    return 1;
   }
  const char *json_conf = argv[1];

  if (!FW_TEST_main_config.load_config(json_conf))
   {
    std::cerr << argv[0] << ":"
              <<  FW_TEST_main_config.get_error_string()
              << std::endl;
    return 2;
   }

#if 0
  FW_TEST_qtl_double.initialize(FW_TEST_main_config.get_conf());
  if (!FW_TEST_qtl_double.parse_ok())
   {
    std::cerr << argv[0] << ": JSON parse error. Exiting." << std::endl;
    return 1;
   }
#endif

  qtl_tick_t start_tick, end_tick = -1;
  qtl_ms_t end_ms = -1;
  if (!FW_TEST_main_config.get_number<qtl_tick_t>(MVM_FM_confattr_StartTick,
                                                  start_tick))
   {
    start_tick = 0;
   }

  if (!FW_TEST_main_config.get_number<qtl_tick_t>(MVM_FM_confattr_EndTick,
                                                  end_tick))
   {
    if (!FW_TEST_main_config.get_number<qtl_tick_t>(MVM_FM_confattr_EndMs,
                                                    end_ms))
     {
      end_tick = 1000000;
      std::cerr << argv[0] << ": Warning. Could not find either the "
                << MVM_FM_confattr_EndTick << " or the "
                << MVM_FM_confattr_EndMs
                << " attribute in " << json_conf
                << ". Using default value: end_tick == "
                << end_tick << "." << std::endl;
     }
   }

  // Main ticker loop
  double in_p = 3000.; // cmh2o
  double atm_p = 984;  // cmh2o
  double loop_p = atm_p;
  const double cmh2o_to_pascal = 98.0665;
  double capacitance = 6;  // liters
  double gas = atm_p*capacitance;
  const double slm_to_m3s = 0.000016667;
  const double psi_to_pascal = 6894.76;
  double resistance = ((4*psi_to_pascal/cmh2o_to_pascal)/(30)); // 4 psi / 30 slm/minute
  std::cerr << "gas: " << gas << " resistance: " << resistance
            << " cmh2o/slm." << std::endl; 
  FW_TEST_main_config.start_time();
  qtl_ms_t oldt = FW_TEST_ms;
  for (FW_TEST_tick = start_tick; ; ++FW_TEST_tick)
   {
    FW_TEST_ms = FW_TEST_main_config.get_scaled_ms();
    if (FW_TEST_ms <= oldt) 
     {
      ::timespec wait;
      wait.tv_sec = 0;
      wait.tv_nsec = 500000 / FW_TEST_main_config.get_scale_factor();
      ::nanosleep(&wait, 0);
      continue;
     }
    if ((end_tick >= 0) && (FW_TEST_tick > end_tick)) break;
    if ((end_ms >= 0) && (FW_TEST_ms > end_ms)) break;
    double inlet_flow = 0.;
    double outlet_flow = 0.;
    double open_fraction = 0.001;
    if (FW_TEST_ms < (end_ms/5)) inlet_flow = ((in_p - loop_p)/resistance)*open_fraction;
    else       outlet_flow = (loop_p - atm_p)/resistance;
    double gas_diff = ((inlet_flow-outlet_flow)/1000 * (FW_TEST_ms - oldt)) * loop_p;
#ifdef DEBUG
    std::cerr << "DEBUG: ms: " << FW_TEST_ms
              << " inlet flow: " << inlet_flow
              << " outlet flow: " << outlet_flow
              << " gas diff: " << gas_diff;
#endif
    gas += gas_diff;
#ifdef DEBUG
    std::cerr << " total gas: " << gas;
#endif
    loop_p = gas/capacitance;
    if (loop_p > in_p)
     {
      loop_p = in_p;
      gas = loop_p*capacitance;
     }
    if (loop_p < atm_p)
     {
      loop_p = atm_p;
      gas = loop_p*capacitance;
     }
#ifdef DEBUG
    std::cerr << " loop_p: " << loop_p 
              << " in_p: " << in_p << std::endl;
#endif
    std::cout << FW_TEST_ms << " " << (loop_p - atm_p) << std::endl;
    oldt = FW_TEST_ms;
   }
  return 0;
}

