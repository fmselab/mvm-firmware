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
        << rapidjson::GetParseError_En(pres.Code()) << ".";
    m_error_string = ers.str();
    return false;
   }
  m_valid = true;
  return true;
}

