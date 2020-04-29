//
// File: mvm_fw_unit_test_config.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 24-Apr-2020 Initial version.
//
// Description:
// Methods to access the JSON configuration file, with a static instance of
// the config handle, as this seems so fashionable around this project.
//

#include <fstream>
#include <sstream>

#include "mvm_fw_unit_test_config.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

std::ostream& operator<< (std::ostream &os, const system_error &serr)
{
  return (os << serr.get_err());
}

bool
mvm_fw_unit_test_config::load_config(const std::string &conf_file)
{
  m_conf_file = conf_file;
  m_valid = false;
  m_error_string.clear();

  std::ifstream ifs(conf_file);
  if (!ifs.good())
   {
    std::ostringstream ers;
    ers << "Error reading from configuration file "
        << conf_file << ": " << system_error() << ".";
    m_error_string = ers.str();
    return false;
   }

  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::ParseResult pres = m_conf.ParseStream(isw);
  if (!pres)
   {
    std::ostringstream ers;
    ers << "Error parsing configuration file "
        << conf_file << ": "
        << rapidjson::GetParseError_En(pres.Code()) 
        << " Offset: " << pres.Offset() << ".";
    m_error_string = ers.str();
    return false;
   }
  m_valid = true;
  return true;
}

void
mvm_fw_unit_test_pflow::m_init()
{
  m_volume = 0.; // Zero volume is atmospheric pressure.
  m_flow = 0.;
  m_last_tick = -1;

  if (!FW_TEST_main_config.get_number<double>("pflow_capacity",
                                              m_capacity))
   {
    m_capacity = 3.; // 'standard' liters, in principle.
                     //  - carefully researched figure.
   }
  if (!FW_TEST_main_config.get_number<double>("pflow_valve_resistance",
                                               m_v_resistance))
   {
    m_v_resistance = 1.; // 1/section of inlet, fundamentally. 
   }
  if (!FW_TEST_main_config.get_number<double>("pflow_mouth_resistance",
                                               m_m_resistance))
   {
    m_m_resistance = 0.2; // 1/section of mouth, fundamentally. 
   }
  if (!FW_TEST_main_config.get_number<double>("overpressure_valve_setting",
                                               m_overpressure))
   {
    m_overpressure = 100.; // arbitrary default - FIXME
   }
  if (!FW_TEST_main_config.get_number<double>("ps1_fraction",
                                               m_ps1_fraction))
   {
    m_ps1_fraction = 0.7; // trying to estimate the pressure fall at PS1
   }
  if (!FW_TEST_main_config.get_number<double>("ps2_fraction",
                                               m_ps2_fraction))
   {
    m_ps2_fraction = 0.1; // trying to estimate the pressure fall at PS2
   }
}

void
mvm_fw_unit_test_pflow::m_evolve(qtl_tick_t tf)
{
  double net_flow = 0.;
  if (tf <= m_last_tick) return;

  // Just a crude finite-difference evolution to establish
  // a plausible interplay between the simulated quantities.
  for (qtl_tick_t t = m_last_tick+1; t<=tf; ++t)
   {
    double in_p = FW_TEST_qtl_double.value("input_line_pressure",t);
    double out_p = FW_TEST_qtl_double.value("env_pressure",t);
    double mask_p = FW_TEST_qtl_double.value("mask_pressure",t);
    
    // admit some volume if input valve open 
    if ((FW_TEST_gdevs[mvm_fw_gpio_devs::BREATHE]) && (in_p > m_p[PS0]))
     {
      double inlet = (m_volume/m_capacity)*((in_p-m_p[PS0])/m_v_resistance)*
                      (FW_TEST_gdevs.get_pv1_fraction());
      m_volume += inlet;
      net_flow += inlet; 
     }
    // expel some volume if output valve open 
    if ((FW_TEST_gdevs[mvm_fw_gpio_devs::OUT_VALVE]) && (m_p[PS0] > out_p))
     {
      m_volume -= (m_volume/m_capacity)*((m_p[PS0]-out_p)/m_v_resistance);
     }
    // deal with autonomous breathing, if present 
    if (!std::isnan(mask_p))
     {
      m_volume += (m_volume/m_capacity)*((mask_p - m_p[PS0])/m_m_resistance);
     }
    if (m_volume < 0)
     {
      // Negative pressure - relief valve kicking in
      net_flow -= m_volume;
      m_volume = 0;
     }
    
    m_p[PS0] = m_volume/m_capacity + out_p;
    if (FW_TEST_gdevs[mvm_fw_gpio_devs::BREATHE])
     {
      m_p[PS1] = (m_p[PS0] - in_p)*m_ps2_fraction;
     }
    else m_p[PS1] = m_p[PS0]; // Should not be so abrupt, really.
    if (m_p[PS1] > m_overpressure)
     {
      // Overpressure valve kicking in
      double overv = (m_volume/m_capacity)*((m_p[PS1]-out_p)/m_v_resistance);
      net_flow -= overv;
     }

    if (FW_TEST_gdevs[mvm_fw_gpio_devs::OUT_VALVE])
     {
      m_p[PS2] = (m_p[PS0] - out_p)*m_ps2_fraction;
     }
    else
     {
      m_p[PS2] = m_p[PS0]; // Should not be so abrupt, really.
     }
   }

  if (tf > m_last_tick)
   {
    m_flow = net_flow / static_cast<double>(tf - m_last_tick);
   }

  m_last_tick = tf;
}

double
mvm_fw_unit_test_pflow::p_value(const std::string &name, qtl_tick_t t)
{
  m_evolve(t);
  if (m_last_tick < t) return std::nan("");
  if (name == "PS0") return m_p[PS0];
  else if (name == "PS1") return m_p[PS1];
  else if (name == "PS2") return m_p[PS2];
  else return std::nan("");
}

double
mvm_fw_unit_test_pflow::f_value(qtl_tick_t t)
{
  m_evolve(t);
  if (m_last_tick < t) return std::nan("");
  return m_flow;
}

