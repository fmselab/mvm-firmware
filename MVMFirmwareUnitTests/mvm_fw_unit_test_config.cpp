//
// File: mvm_fw_unit_test_config.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 24-Apr-2020 Initial version.
// 18-May-2020 Added possibility to include other config files.
//
// Description:
// Methods to access the JSON configuration file, with a static instance of
// the config handle, as this seems so fashionable around this project.
//

#include <fstream>
#include <sstream>

#include "mvm_fw_unit_test_config.h"

#ifdef JSON_INSTEAD_OF_YAML
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#else
#include <yaml-cpp/yaml.h>
#endif

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

#ifdef JSON_INSTEAD_OF_YAML
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::ParseResult pres = m_conf.ParseStream(isw);
  if (!pres)
#else
  m_conf = YAML::Load(ifs);
  if (!m_conf)
#endif
   {
    std::ostringstream ers;
    ers << "Error parsing configuration file "
        << conf_file << ": "
#ifdef JSON_INSTEAD_OF_YAML
        << rapidjson::GetParseError_En(pres.Code())
        << " Offset: " << pres.Offset()
#endif
        << ".";
    m_error_string = ers.str();
    return false;
   }
  m_valid = true;
  std::string includef;
  if (get_string("include", includef))
   {
    mvm_fw_unit_test_config newc(includef);
    other_confs.insert(other_confs.end(), newc.get_other_confs().begin(),
                                          newc.get_other_confs().end());
    newc.clear_other_confs();
    other_confs.push_back(newc);
   }
  return true;
}

int
mvm_fw_unit_test_config::load_command_timeline(mvm_fw_test_cmds_t &ctl,
                                               const std::string &name) const
{
  int ret = -1;

  const char *cname=name.c_str();
#ifdef JSON_INSTEAD_OF_YAML
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
#else
  YAML::Node a;
  if (a = m_conf[cname])
   {
    if (!a.IsSequence()) return ret;
    ret = 0;
    for (std::size_t i = 0; i < a.size(); i++)
     {
      YAML::Node v(a[i]);
      if (!(v.IsMap())) continue;
      YAML::Node vt;
      if (!(vt = v["t"])) continue;
      YAML::Node vc;
      if (!(vc = v["c"])) continue;
      if (!(vt.IsScalar())) continue;
      if (!(vc.IsScalar())) continue;
      qtl_tick_t ts = vt.as<qtl_tick_t>();
      std::string cmd = (vc.as<std::string>());
      ctl.insert(std::make_pair(ts, cmd));
      ret++;
     }
   }
#endif

  otherf_container::const_iterator oit;
  otherf_container::const_iterator oend = other_confs.end();
  for (oit = other_confs.begin(); oit != oend; ++oit)
   {
    oit->load_command_timeline(ctl, name);
   }
  return ret;
}

void
mvm_fw_unit_test_pflow::init()
{
  m_last_ms = FW_TEST_ms;

  if (!FW_TEST_main_config.get_number<double>("pflow_capacity",
                                              m_capacity))
   {
    m_capacity = 3.; // 'standard' liters, in principle.
                     //  - carefully researched figure.
   }

  if (!FW_TEST_main_config.get_number<double>("pflow_lung_k",
                                              m_lung_k))
   {
    m_lung_k = 1/50.;
   }

  if (!FW_TEST_main_config.get_number<double>("pflow_lung_max_capacity",
                                              m_lung_max_capacity))
   {
    m_lung_max_capacity = 0.6;
   }

  if (!FW_TEST_main_config.get_number<double>("pflow_in_valve_resistance",
                                               m_in_v_resistance))
   {
    m_in_v_resistance = 0.6;
   }
  if (!FW_TEST_main_config.get_number<double>("pflow_out_valve_resistance",
                                               m_out_v_resistance))
   {
    m_out_v_resistance = 0.003;
   }
  if (!FW_TEST_main_config.get_number<double>("pflow_mouth_resistance",
                                               m_m_resistance))
   {
    m_m_resistance = 1.;
   }
  if (!FW_TEST_main_config.get_number<double>("peep_setting",
                                               m_peep))
   {
    m_peep = 8.;
   }
  if (!FW_TEST_main_config.get_number<double>("overpressure_valve_setting",
                                               m_overpressure))
   {
    m_overpressure = 80.; // arbitrary default - FIXME
   }
  if (!FW_TEST_main_config.get_number<double>("venturi_flow_at_1_psi_drop",
                                               m_venturi_flow_at_1_psi_drop))
   {
    m_venturi_flow_at_1_psi_drop = 33.7; // Crudely assume it's linear.
   }
   m_cur_p = FW_TEST_qtl_double.value("env_pressure",FW_TEST_ms);
   m_in_flow = 0;
   m_v_flow = 0;
   m_p[PI3] = m_cur_p;
   m_p[PI2] = m_cur_p;
   m_p[PI1] = 0; // Venturi deltaP
   m_old_c = FW_TEST_qtl_double.value("patient_capacity",FW_TEST_ms);
   if (std::isnan(m_old_c))
    {
     m_gas = m_capacity * (m_cur_p + m_peep); // Atmospheric pressure level + PEEP. Gas const == 1
     m_old_c = m_capacity;
    }
   else
    {
     m_old_c += m_capacity;
     m_gas = m_old_c * m_cur_p;
    }
   m_gas_rest = m_gas;
   m_inited = true;
}

void
mvm_fw_unit_test_pflow::m_evolve(qtl_ms_t tf)
{
  double net_in_flow = 0.;
  double net_v_flow = 0.;
  if (tf <= m_last_ms) return;

  // Just a crude finite-difference evolution to establish
  // a plausible interplay between the simulated quantities.
  double in_p, pat_c, cur_c;
  for (qtl_ms_t t = m_last_ms+1; t<=tf; ++t)
   {
    double cur_v_flow  = (m_v_flow * (tf-t) + net_v_flow)
                         / static_cast<double>(tf - m_last_ms);
    in_p = FW_TEST_qtl_double.value("input_line_pressure",t);
    m_cur_p = FW_TEST_qtl_double.value("env_pressure",t);
    // the effect of autonomous breathing is an increase in
    // the 'pipe' capacity.
    cur_c = m_capacity;
    pat_c = FW_TEST_qtl_double.value("patient_capacity",t);
    if (!std::isnan(pat_c)) cur_c += pat_c;
    else
     {
      double inflation_fraction = (m_lung_k*(m_gas - m_gas_rest));
      if (inflation_fraction < 0) inflation_fraction = 0;
      if (inflation_fraction > 1) inflation_fraction = 1;
      cur_c += m_lung_max_capacity * inflation_fraction;
     }

    // admit some volume of gas if input valve open
    double pv1_open_fraction = FW_TEST_gdevs.get_pv1_fraction();
    if ((pv1_open_fraction > 0) && (in_p > m_p[PI3]))
     {
      double inlet = ((in_p-m_p[PI3])/m_in_v_resistance)*
                     (FW_TEST_gdevs.get_pv1_fraction()) / 1000.; // per 1 ms
      m_gas += inlet;
      inlet /= m_p[PI2];
      net_in_flow += inlet;
      net_v_flow += inlet;
     }
    // exchange some volume if output valve open
    // Valve FALSE is open and TRUE is closed...
    if (!FW_TEST_gdevs[mvm_fw_gpio_devs::OUT_VALVE])
     {
      double exch = ((m_cur_p + m_peep - m_p[PI2])/m_out_v_resistance) / 1000.; // per 1 ms
      m_gas += exch;
      exch /= m_p[PI2];
      net_v_flow += exch;
     }
    m_p[PI2] = m_gas / cur_c + cur_v_flow * m_m_resistance;
    if (pv1_open_fraction>0)
     {
      m_p[PI3] = m_p[PI2] +
            (net_in_flow/t)*m_in_v_resistance*FW_TEST_gdevs.get_pv1_fraction();
     }
    else m_p[PI3] = m_p[PI2];
    if ((m_p[PI3] - m_cur_p)  > m_overpressure)
     {
      // Overpressure valve kicking in
      double overv = ((m_p[PI3]-m_cur_p)/m_out_v_resistance) / 1000.; // per 1 ms
      m_gas -= overv;
      overv /= m_p[PI2];
      net_in_flow -= overv;
     }
    if ((FW_TEST_gdevs[mvm_fw_gpio_devs::OUT_VALVE]) && (m_p[PI3] < m_cur_p))
     {
      // Negative pressure && out_valve closed - relief valve kicking in
      double inlet = ((m_cur_p-m_p[PI3])/m_out_v_resistance) / 1000.; // per 1 ms
      m_gas += inlet;
      inlet /= m_p[PI2];
      net_in_flow += inlet;
     }
   }

  if (!std::isnan(pat_c)) net_v_flow += (m_old_c - cur_c); // liters
  m_old_c = cur_c;
  m_in_flow = net_in_flow / static_cast<double>(tf - m_last_ms);
  m_v_flow  = net_v_flow / static_cast<double>(tf - m_last_ms);
  // Estimate Venturi deltaP in PSI per liters/minute at first order.
  m_p[PI1] = (m_v_flow*60000.)  / m_venturi_flow_at_1_psi_drop;

#ifdef DEBUG
  std::cerr << "DEBUG: deltaT:" << (tf - m_last_ms) << ", net_in_flow:"
            << net_in_flow << ", net_v_flow:" << net_v_flow
            << ", m_p[PI1](Venturi):" << m_p[PI1]
            << ", m_in_flow:" << m_in_flow << " x60k:" << (m_in_flow*60000)
            << ", m_v_flow:" << m_v_flow << std::endl;
  std::cerr << "DEBUG: m_gas:" << m_gas << ", cur_c:" << cur_c << std::endl;
  std::cerr << "DEBUG: ms:" << FW_TEST_ms << ", pin:" << in_p
            << ", pLoop:" << m_p[PI3] << ", pPatient:"
            << m_p[PI2] << ", pventuri:" << m_p[PI1]
            << ", pout:" << m_cur_p
            << " in_flow:" << m_in_flow
            << " v_flow:" << m_v_flow << std::endl;
#endif

  m_last_ms = tf;
}

double
mvm_fw_unit_test_pflow::p_value(const std::string &name, qtl_ms_t t)
{
  double enable;
  enable = FW_TEST_qtl_double.value(name+"_enable", FW_TEST_ms);
  if (!std::isnan(enable) && (enable == 0)) return std::nan("");

  double ovval;
  ovval = FW_TEST_qtl_double.value(name+"_override", FW_TEST_ms);
  if (!std::isnan(ovval)) return ovval;

  if (!m_inited) return std::nan("");
  m_evolve(t);
  if (m_last_ms < t) return std::nan("");
  if (name == "PI3") return (m_p[PI3] - m_cur_p);
  else if (name == "PI2") return (m_p[PI2] - m_cur_p); // Would be out of range
                                                       // for atm. pressure.
  else if (name == "PI1") return m_p[PI1];
  else return std::nan("");
}

double
mvm_fw_unit_test_pflow::in_f_value(qtl_ms_t t)
{
  double enable;
  enable = FW_TEST_qtl_double.value("in_flow_enable", FW_TEST_ms);
  if (!std::isnan(enable) && (enable == 0)) return std::nan("");

  double ovval;
  ovval = FW_TEST_qtl_double.value("in_flow_override", FW_TEST_ms);
  if (!std::isnan(ovval)) return ovval;

  if (!m_inited) return std::nan("");
  m_evolve(t);
  if (m_last_ms < t) return std::nan("");
  return (m_in_flow*60000); // liters per minute;
}

double
mvm_fw_unit_test_pflow::v_f_value(qtl_ms_t t)
{
  double enable;
  enable = FW_TEST_qtl_double.value("venturi_flow_enable", FW_TEST_ms);
  if (!std::isnan(enable) && (enable == 0)) return std::nan("");

  double ovval;
  ovval = FW_TEST_qtl_double.value("venturi_flow_override", FW_TEST_ms);
  if (!std::isnan(ovval)) return ovval;

  if (!m_inited) return std::nan("");
  m_evolve(t);
  if (m_last_ms < t) return std::nan("");
  return (m_v_flow*60000); // liters per minute;
}

