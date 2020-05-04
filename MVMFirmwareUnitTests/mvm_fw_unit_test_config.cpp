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

int
mvm_fw_unit_test_config::load_command_timeline(mvm_fw_test_cmds_t &ctl,
                                               const std::string &name)
{
  int ret = -1;

  const char *cname=name.c_str();
  if (m_conf.HasMember(cname))
   {
    const rapidjson::Value& a(m_conf[cname]);
    if (!a.IsArray()) return ret;
    ret = 0;
    for (rapidjson::SizeType i = 0; i < a.Size(); i++)
     {
      const rapidjson::Value& v(a[i]);
      if (!(v.IsObject())) continue;
      if (!(v.HasMember("t"))) continue;
      if (!(v.HasMember("c"))) continue;
      const rapidjson::Value& vt(v["t"]);
      const rapidjson::Value& vc(v["c"]);
      if (!(vt.IsNumber())) continue;
      if (!(vc.IsString())) continue;
      qtl_tick_t ts = vt.Get<qtl_tick_t>();
      std::string cmd(vc.GetString());
      ctl.insert(std::make_pair(ts, cmd));
      ret++;
     }
   }

  return ret;
}

void
mvm_fw_unit_test_pflow::init()
{
  m_volume = 0.; // Zero volume is atmospheric pressure.
  m_flow = 0.;
  m_last_ms = FW_TEST_ms;

  if (!FW_TEST_main_config.get_number<double>("pflow_capacity",
                                              m_capacity))
   {
    m_capacity = 3.; // 'standard' liters, in principle.
                     //  - carefully researched figure.
   }
  if (!FW_TEST_main_config.get_number<double>("pflow_valve_resistance",
                                               m_v_resistance))
   {
    m_v_resistance = 10.; // 1/section of inlet, fundamentally. 
   }
  if (!FW_TEST_main_config.get_number<double>("pflow_mouth_resistance",
                                               m_m_resistance))
   {
    m_m_resistance = 2; // 1/section of mouth, fundamentally. 
   }
  if (!FW_TEST_main_config.get_number<double>("overpressure_valve_setting",
                                               m_overpressure))
   {
    m_overpressure = 100.; // arbitrary default - FIXME
   }
  if (!FW_TEST_main_config.get_number<double>("ps1_fraction",
                                               m_ps1_fraction))
   {
    m_ps1_fraction = 0.8; // trying to estimate the pressure fall at PS1
   }
  if (!FW_TEST_main_config.get_number<double>("ps2_fraction",
                                               m_ps2_fraction))
   {
    m_ps2_fraction = 0.1; // trying to estimate the pressure fall at PS2
   }
   double cur_p = FW_TEST_qtl_double.value("env_pressure",FW_TEST_ms);
   m_flow = 0;
   m_p[PS0] = cur_p;
   m_p[PS1] = cur_p;
   m_p[PS2] = cur_p;
   m_inited = true;
}

void
mvm_fw_unit_test_pflow::m_evolve(qtl_ms_t tf)
{
  double net_flow = 0.;
  if (tf <= m_last_ms) return;

  // Just a crude finite-difference evolution to establish
  // a plausible interplay between the simulated quantities.
  double in_p, out_p, mask_p;
  for (qtl_ms_t t = m_last_ms+1; t<=tf; ++t)
   {
    in_p = FW_TEST_qtl_double.value("input_line_pressure",t);
    out_p = FW_TEST_qtl_double.value("env_pressure",t);
    mask_p = FW_TEST_qtl_double.value("mask_pressure",t);
    
    // admit some volume if input valve open 
    double pv1_open_fraction = FW_TEST_gdevs.get_pv1_fraction();
    if ((pv1_open_fraction > 0) && (in_p > m_p[PS0]))
     {
      double inlet = (1/m_capacity)*((in_p-m_p[PS0])/m_v_resistance)*
                      (FW_TEST_gdevs.get_pv1_fraction());
      m_volume += inlet;
      net_flow += inlet; 
     }
    // expel some volume if output valve open 
    // Valve FALSE is open and TRUE is closed...
    if ((!FW_TEST_gdevs[mvm_fw_gpio_devs::OUT_VALVE]) && (m_p[PS0] > out_p))
     {
      m_volume -= ((m_p[PS0]-out_p)/m_v_resistance);
     }
    // deal with autonomous breathing, if present 
    if (!std::isnan(mask_p))
     {
      m_volume += ((mask_p - m_p[PS0])/m_m_resistance);
     }
    if (m_volume < 0)
     {
      // Negative pressure - relief valve kicking in
      net_flow -= m_volume;
      m_volume = 0;
     }
    
    if (!std::isnan(mask_p))
     {
      m_p[PS0] = mask_p;
     }
    else
     {
      m_p[PS0] = m_volume/m_capacity + out_p;
     }

    if (pv1_open_fraction>0)
     {
      m_p[PS1] = (in_p - m_p[PS0])*m_ps1_fraction + out_p;
     }
    else m_p[PS1] = m_p[PS0]; // Should not be so abrupt, really.
    if (m_p[PS1] > m_overpressure)
     {
      // Overpressure valve kicking in
      double overv = ((m_p[PS1]-out_p)/m_v_resistance);
      net_flow -= overv;
     }

    if (!(FW_TEST_gdevs[mvm_fw_gpio_devs::OUT_VALVE])) 
     {
      // Valve open.
      m_p[PS2] = (m_p[PS0] - out_p)*m_ps2_fraction + out_p;
     }
    else
     {
      m_p[PS2] = m_p[PS0]; // Should not be so abrupt, really.
     }
   }

  m_flow = net_flow / static_cast<double>(tf - m_last_ms) * 1000; // cm**3

#ifdef DEBUG
  std::cerr << "DEBUG: tck:" << FW_TEST_tick << ", pin:" << in_p 
            << ", p0:" << m_p[PS0] << ", p1:" 
            << m_p[PS1] << ", p2:" << m_p[PS2] << ", pout:" << out_p
            << " flow:" << m_flow << std::endl;
#endif

  m_last_ms = tf;
}

double
mvm_fw_unit_test_pflow::p_value(const std::string &name, qtl_ms_t t)
{
  if (!m_inited) return std::nan("");
  m_evolve(t);
  if (m_last_ms < t) return std::nan("");
  if (name == "PS0") return m_p[PS0];
  else if (name == "PS1") return m_p[PS1];
  else if (name == "PS2") return m_p[PS2];
  else return std::nan("");
}

double
mvm_fw_unit_test_pflow::f_value(qtl_ms_t t)
{
  if (!m_inited) return std::nan("");
  m_evolve(t);
  if (m_last_ms < t) return std::nan("");
  return m_flow;
}

