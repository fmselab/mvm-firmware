//
// File: quantity_timelines.hpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 23-Apr-2020 Initial version.
//
// Description:
// Quick framework to configure the time variation of a generic collection
// of quantities, configured in a layered fashion via JSON.
//

#ifndef _QUANTITY_TIMELINES_HPP
#define _QUANTITY_TIMELINES_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>
#include <stdint.h> // <cstdint> is C++11.

#ifdef JSON_INSTEAD_OF_YAML
#include <rapidjson/document.h>
#include <rapidjson/error/en.h> 
#include <rapidjson/istreamwrapper.h>
#else
#include <yaml-cpp/yaml.h>
#endif

#include <exprtk.hpp>

typedef int64_t qtl_tick_t; 
typedef int64_t qtl_ms_t; 

inline std::string
mvm_fw_unit_test_dirname(const std::string &path)
{
  std::string ret;

  std::size_t ls = path.find_last_of("/\\");
  if (ls != std::string::npos)
   {
    ret = path;
    ret.erase(ls+1);
   }
  return ret;
}

inline bool
mvm_fw_unit_test_not_absolute(const std::string &path)
{
  std::size_t fs = path.find_first_of("/\\");
  if (fs == 0) return false;
  return true;
}

template<typename TNUM>
class qtl_config_blob: public exprtk::ifunction<TNUM>
{
  public:
    qtl_config_blob(const char *name): exprtk::ifunction<TNUM>(1),
                                       m_name(name), m_depth(0),
                                       m_valid(false), m_should_repeat(false),
                                       m_is_constant(false)
    {}
    qtl_config_blob(const std::string &name): exprtk::ifunction<TNUM>(1),
                                       m_name(name), m_depth(0),
                                       m_valid(false), m_should_repeat(false),
                                       m_is_constant(false)
    {}
    ~qtl_config_blob() {}

    const TNUM inval = static_cast<TNUM>(std::nan(""));
    typedef std::unordered_map<std::string, qtl_config_blob<TNUM> > function_map_t;

    bool initialize(qtl_ms_t start, 
                    std::vector<TNUM> &data,
                    int depth = 0,
                    bool should_repeat = false,
                    qtl_ms_t rep_start=0, qtl_ms_t rep_end=0)
     {
      m_ms_start = start;
      m_ms_end   = start + data.size();
      m_depth = depth;
      m_values = data; 
      m_should_repeat = should_repeat;
      m_repeat_start = rep_start;
      m_repeat_end = rep_end;
      m_valid = true;
      return m_valid;
     }

    bool initialize(qtl_ms_t start, 
                    const char *data, const char *delimiter=",",
                    int depth = 0,
                    bool should_repeat = false,
                    qtl_ms_t rep_start=0, qtl_ms_t rep_end=0)
     {
      char *saveptr;
      char *spt = ::strtok_r(data, delimiter, &saveptr);
      m_ms_start = m_ms_end = start;
      m_depth = depth;
      m_should_repeat = should_repeat;
      m_repeat_start = rep_start;
      m_repeat_end = rep_end;
      m_valid = m_init_from_cstr(data, delimiter);
      return m_valid;
     }

    bool initialize(qtl_ms_t start, qtl_ms_t end,
                    TNUM const_val,
                    int depth = 0)
     {
      m_ms_start = start;
      m_ms_end   = end;
      m_depth = depth;
      m_is_constant = true;
      m_const_value = const_val; 
      m_valid = true;
      return m_valid;
     }

    bool initialize(qtl_ms_t start, 
                    std::string data, const char *delimiter=",",
                    int depth = 0,
                    bool should_repeat = false,
                    qtl_ms_t rep_start=0, qtl_ms_t rep_end=0)
     {
      return initialize(start, should_repeat, rep_start, rep_end,
                        data.c_str(), delimiter);
     }

    /* Generic initialization */
    void set_data(qtl_ms_t start, 
                  int depth = 0,
                  bool should_repeat = false,
                  qtl_ms_t rep_start=0, qtl_ms_t rep_end=0)
     {
      m_ms_start = m_ms_end = start;
      m_depth = depth;
      m_should_repeat = should_repeat;
      m_repeat_start = rep_start;
      m_repeat_end = rep_end;
     }

    bool operator<(const qtl_config_blob &other)
     {
      return (this.depth > other.depth);
     }

    const std::vector<std::string> &get_unknown_function_list() const
     {
      return m_dep_fun;
     }

    const std::vector<std::string> &get_dependent_blob_list() const
     {
      return m_dep_fun;
     }

    const std::string &get_name() const
     {
      return m_name;
     }

    const std::vector<TNUM> &get_values() const
     {
      return m_values;
     }

    bool is_constant() const
     {
      if (m_valid) return m_is_constant;
      else return false;
     }

    qtl_ms_t get_const_value(TNUM &val) const
     {
      if (!m_valid) return 0;
      val = m_const_value;
      return (m_ms_end - m_ms_start);
     }

    /* Initialize from expr */
    bool initialize(qtl_ms_t start, qtl_ms_t end,
                 const std::string &expr,
                 int depth = 0,
                 bool should_repeat = false,
                 qtl_ms_t rep_start=0, qtl_ms_t rep_end=0);


    bool resolve(function_map_t &mfun);

    void add_dep_blob(const std::string &news)
     {
      m_dep_blobs.push_back(news);
     }

    std::vector<qtl_config_blob <TNUM> > other_blobs;
    void add_other_blob(const qtl_config_blob<TNUM> &newb)
     {
      if (newb.get_name() != m_name) return;
      other_blobs.push_back(newb);
     }

    inline TNUM evaluate(qtl_ms_t t) const
     {
      if (! m_valid) return inval;
      if (m_is_constant)
       { 
        if ((m_ms_start == m_ms_end) ||
            ((t >= m_ms_start) && (t < m_ms_end))) return m_const_value;
        else return inval;
       }

      bool valid = true;
      qtl_ms_t idx;
      if (m_should_repeat)
       {
        qtl_ms_t rept;
        if (m_repeat_start != m_repeat_end)
         {
          rept = (t - m_repeat_start);
          if ((t < m_repeat_start) || (t >= m_repeat_end)) valid = false;
          else idx = rept%(m_ms_end - m_ms_start);
         }
        else
         {
          rept = t;
         }
        idx = rept%(m_ms_end - m_ms_start);
        if (rept < 0) { idx += (m_ms_end - m_ms_start); }
       }
      else
       {
        if ((t < m_ms_start) || (t >= m_ms_end)) valid = false;
        else idx = (t - m_ms_start);
       }
      if (valid && (idx >= m_values.size())) valid = false;
      if (valid) return m_values[idx];

      for (int i=0; i < other_blobs.size(); ++i)
       {
        TNUM tret = other_blobs[i](t);
        if (! std::isnan(tret)) return tret;
       }
      return inval;
     }

    bool valid() const { return m_valid; }

    TNUM operator()(const TNUM &t) 
     {
      return evaluate(static_cast<qtl_ms_t>(t));
     }

    TNUM operator()(qtl_ms_t t) const
     {
      return evaluate(t);
     }

  private:
    std::string m_name; 
    int         m_depth;
    bool        m_valid;
    qtl_ms_t  m_ms_start, m_ms_end;
    bool        m_should_repeat;
    qtl_ms_t  m_repeat_start, m_repeat_end;
    bool        m_is_constant;
    TNUM        m_const_value;
    std::vector<TNUM> m_values;
    std::string m_expr;
    std::vector<std::string> m_dep_fun;
    std::vector<std::string> m_dep_blobs;

    inline bool m_init_from_cstr(const char *data, const char *delimiter=",")
     {
      char *saveptr;
      char *spt = ::strtok_r(data, delimiter, &saveptr);
      if (spt == NULL) return false;
      while (spt != NULL)
       {
        std::istringstream ivs(spt);
        TNUM dat;
        ivs >> dat;
        if (ivs.good())
         {
          m_values.push_back(dat);
          ++m_ms_end; 
         }
        spt = ::strtok_r(data, delimiter, &saveptr);
       }
      return true;
     }
};


template<typename TNUM>
bool
qtl_config_blob<TNUM>::initialize(qtl_ms_t start, qtl_ms_t end,
                                  const std::string &expr_str,
                                  int depth,
                                  bool should_repeat,
                                  qtl_ms_t rep_start,
                                  qtl_ms_t rep_end)
{
  m_ms_start = start;
  m_ms_end = end;
  m_depth = depth;
  m_should_repeat = should_repeat;
  m_repeat_start = rep_start;
  m_repeat_end = rep_end;

  TNUM t;
  exprtk::symbol_table<TNUM> unknown_sym_table;
  exprtk::symbol_table<TNUM> sym_table;
  sym_table.add_variable("t",t);

  exprtk::expression<TNUM> expr;
  expr.register_symbol_table(unknown_sym_table);
  expr.register_symbol_table(sym_table);

  exprtk::parser<TNUM> parser;
  parser.enable_unknown_symbol_resolver();

  if (parser.compile(expr_str,expr))
   {
    unknown_sym_table.get_variable_list(m_dep_fun);

    std::vector<std::string>::const_iterator it;
    std::vector<std::string>::const_iterator end = m_dep_fun.end();
    m_expr = expr_str;
    return true;
   }
  else
   {
    std::cerr << "DEBUG: failure parsing expr <" << expr_str << ">" << std::endl;
    m_expr.clear();
   } 
  return false;
}
 
template<typename TNUM>
bool
qtl_config_blob<TNUM>::resolve(function_map_t &mfun)
{
  if (m_valid) return true;

  if ((m_dep_blobs.size() > 0) && (mfun.size() > 0)) 
   {
    // Dependent blob case
    std::vector<std::string>::const_iterator it;
    std::vector<std::string>::const_iterator bend = m_dep_blobs.end();
    typename std::unordered_map<std::string, qtl_config_blob<TNUM> >::const_iterator fbend = mfun.end();
    bool all_valid = true;
    for (it = m_dep_blobs.begin(); it != bend; ++it)
     {
      typename std::unordered_map<std::string, qtl_config_blob<TNUM> >::const_iterator bfnd;
      bfnd = mfun.find(*it);
      if (bfnd != fbend) 
       {
        if (!bfnd->second.valid())
         {
          all_valid = false;
          break;
         }
       }
     }
    if (!all_valid) return m_valid;;

    for (it = m_dep_blobs.begin(); it != bend; ++it)
     {
      typename std::unordered_map<std::string, qtl_config_blob<TNUM> >::const_iterator bfnd;
      bfnd = mfun.find(*it);
      if (bfnd != fbend) 
       {
        if (bfnd->second.is_constant())
         {
          TNUM value;
          qtl_ms_t size = bfnd->second.get_const_value(value);
          m_values.insert(m_values.end(), size, value);
         }
        else
         {
          m_values.insert(m_values.end(),
            bfnd->second.get_values().begin(), bfnd->second.get_values().end());
         }
        m_ms_end = m_ms_start + m_values.size();
        m_valid = true;
       }
     }
   }
  else
   {
    // Dependent function case
    TNUM tt;
    exprtk::symbol_table<TNUM> sym_table;
    sym_table.add_variable("t",tt);
  
    std::vector<std::string>::const_iterator it;
    std::vector<std::string>::const_iterator fend = m_dep_fun.end();
    typename function_map_t::const_iterator fmend = mfun.end();
    for (it = m_dep_fun.begin(); it != fend; ++it)
     {
      typename function_map_t::iterator ffnd;
      ffnd = mfun.find(*it);
      if (ffnd != fmend) 
       {
        if (!(ffnd->second.valid())) return m_valid;
        sym_table.add_function(ffnd->first,ffnd->second);
       }
     }
     exprtk::expression<TNUM> expr;
     expr.register_symbol_table(sym_table);
     exprtk::parser<TNUM> parser;
  
     if (parser.compile(m_expr,expr))
      {
       for (qtl_ms_t t=m_ms_start; t<=m_ms_end; ++t)
        {
         tt = static_cast<TNUM>(t);
         m_values.push_back(expr.value());
        }
       m_valid = true;
      }
    }

  return m_valid;
}

static const char *default_head_el = "qtl_timelines";

template<typename TNUM>
class quantity_timelines
{
  public:

    typedef std::unordered_map<std::string, qtl_config_blob<TNUM> > qtl_blob_map_t;
    typedef std::map<int, qtl_blob_map_t> qtl_blob_container_t;

    const TNUM inval = static_cast<TNUM>(std::nan(""));

    quantity_timelines() : m_parse_ok(false), m_count(0) {}
    quantity_timelines(const char *config_filename,
                       const char *head_el=default_head_el) : m_parse_ok(false), m_count(0)
     {
      initialize(config_filename, head_el);
     }
    quantity_timelines(std::string config_filename,
                       std::string head_el=default_head_el) : m_parse_ok(false), m_count(0)
     {
      initialize(config_filename.c_str(), head_el.c_str());
     }
    ~quantity_timelines() {}

    void initialize(const char *config_filename,
                    const char *head_el=default_head_el);
#ifdef JSON_INSTEAD_OF_YAML
    void initialize(const rapidjson::Document &d,
#else
    // WARNING: even if this method is read-only on the YAML node
    //          declaring it 'const' will lead to state corruption.
    void initialize(YAML::Node &n,
#endif
                    const char *head_el = default_head_el,
                    const std::string &confdir = "");

    bool parse_ok() const { return m_parse_ok; }
    int  count() const { return m_count; }

    TNUM value(const std::string &name, qtl_ms_t t)
     {
      TNUM res = inval;
      if (!m_parse_ok) return res;

      typename qtl_blob_container_t::const_iterator bit;        
      typename qtl_blob_container_t::const_iterator bend = m_blobs.end();        
      for (bit = m_blobs.begin(); bit != bend; ++bit)
       {
        typename qtl_blob_map_t::const_iterator bn;
        bn = bit->second.find(name);
        if (bn != bit->second.end())
         {
          res=bn->second(t);
          if (! std::isnan(res)) break;
         }
       }
      return res;
     }

  private:

    qtl_blob_container_t m_blobs;
    bool m_parse_ok;
    int m_count;
   
};

template<typename TNUM>
void
quantity_timelines<TNUM>::initialize(const char *config_filename,
                                     const char *head_el)
{
  std::ifstream ifs(config_filename);
  if (!ifs.good()) return;

#ifdef JSON_INSTEAD_OF_YAML
  rapidjson::IStreamWrapper isw(ifs);
 
  rapidjson::Document d;
  rapidjson::ParseResult pres = d.ParseStream(isw);
  if (pres)
#else
  YAML::Node d = YAML::Load(ifs);
  if (d)
#endif
   {
    initialize(d, head_el,
               mvm_fw_unit_test_dirname(std::string(config_filename)));
   }
  else
   {
      std::cerr << "DEBUG: Parse Error! " 
#ifdef JSON_INSTEAD_OF_YAML
                << rapidjson::GetParseError_En(pres.Code())
#endif
                << std::endl;
   }
}

template<typename TNUM>
void
#ifdef JSON_INSTEAD_OF_YAML
quantity_timelines<TNUM>::initialize(const rapidjson::Document &d,
#else
quantity_timelines<TNUM>::initialize(YAML::Node &n,
#endif
                            const char *head_el, const std::string &confdir)
{
#ifdef JSON_INSTEAD_OF_YAML
  if (d.HasMember(head_el))
#else
  YAML::Node a;
  if (a = n[head_el])
#endif
   {
    m_parse_ok = true;
#ifdef JSON_INSTEAD_OF_YAML
    const rapidjson::Value& a(d[head_el]);
    if (!a.IsArray()) return;
    for (rapidjson::SizeType i = 0; i < a.Size(); i++)
     {
      if (!(a[i].IsObject())) continue;
      const rapidjson::Value& m(a[i]); 
#ifdef DEBUG
      for (rapidjson::Value::ConstMemberIterator itr = m.MemberBegin();
           itr != m.MemberEnd(); ++itr)
       {
        if (itr->value.IsString()) //if array object value is string
          std::cerr << itr->name.GetString() << ": " <<  itr->value.GetString();
        else if (itr->value.IsBool()) //if bool
          std::cerr << itr->name.GetString() << ": " << itr->value.GetBool();
        else if (itr->value.IsNumber()) //if array object value is integer
          std::cerr << itr->name.GetString() << ": "  <<  itr->value.GetFloat();
        std::cerr << std::endl;
       }
#endif // DEBUG
#else //!JSON_INSTEAD_OF_YAML
    if (!a.IsSequence()) return;
    for (int i = 0; i < a.size(); i++)
     {
      YAML::Node m(a[i]); 
      if ((!m) || (!m.IsMap())) continue;
#endif //!JSON_INSTEAD_OF_YAML
#ifdef JSON_INSTEAD_OF_YAML
      if (!m.HasMember("name"))
       {
        if (!m.HasMember("include")) continue;
        const rapidjson::Value& inam(m["include"]); 
        if (!(inam.IsString())) continue;
        std::string ifil(inam.GetString());
        if (mvm_fw_unit_test_not_absolute(ifil))
         {
          // Relative path ? Read file relative to original config dir.
          ifil = confdir + ifil;
         }
        std::string idir = mvm_fw_unit_test_dirname(ifil);
        initialize(inam.GetString(), head_el, idir);
        continue;
       }
      if (!m.HasMember("start")) continue;
      const rapidjson::Value& vnam(m["name"]); 
      if (!(vnam.IsString())) continue;
      const rapidjson::Value& sstart(m["start"]); 
      if (!(sstart.IsNumber())) continue;
      qtl_config_blob<TNUM> newblob(vnam.GetString());
      int depth = 0;
      bool repeat = false;
      qtl_ms_t start, end, repeat_start, repeat_end;
      start = end = repeat_start = repeat_end = 0;
      start = sstart.Get<qtl_ms_t>();
      if (m.HasMember("depth"))
       {
        const rapidjson::Value& dnam(m["depth"]);
        if (dnam.IsNumber()) depth = dnam.GetInt();
       }
      if (m.HasMember("end"))
       {
        const rapidjson::Value& enam(m["end"]);
        if (enam.IsNumber()) end = enam.Get<qtl_ms_t>();
        if (end != start) ++end; // Upper boundary is not included
                                 // in the interval. But equal values
                                 // mean forever. 
       }
      if (m.HasMember("repeat"))
       {
        const rapidjson::Value& rnam(m["repeat"]);
        if (rnam.IsBool()) repeat = rnam.GetBool();
       }
      if (m.HasMember("repeat_start"))
       {
        const rapidjson::Value& snam(m["repeat_start"]);
        if (snam.IsNumber()) repeat_start = snam.Get<qtl_ms_t>();
       }
      if (m.HasMember("repeat_end"))
       {
        const rapidjson::Value& enam(m["repeat_end"]);
        if (enam.IsNumber()) repeat_end = enam.Get<qtl_ms_t>();
       }
      if (m.HasMember("value"))
       {
        TNUM value;
        const rapidjson::Value& dnam(m["value"]);
        if (dnam.IsNumber()) value = dnam.Get<TNUM>();
        newblob.initialize(start, end, value, depth); 
       }
      else if (m.HasMember("expr"))
       {
        const rapidjson::Value& enam(m["expr"]);
        if (enam.IsString()) newblob.initialize(start, end,
                             enam.GetString(), depth, repeat,
                             repeat_start, repeat_end );
       }
      else if (m.HasMember("compose"))
       {
        const rapidjson::Value& ca(m["compose"]);
        if (ca.IsArray())
         {
          for (rapidjson::SizeType cai = 0; cai < ca.Size(); cai++)
           {
            if (ca[cai].IsString())
             {
              newblob.add_dep_blob(ca[cai].GetString());
             }
           }
          newblob.set_data(start, depth, repeat,
                           repeat_start, repeat_end);
         }
       }
      else if (m.HasMember("values"))
       {
        const rapidjson::Value& ca(m["values"]);
        if (ca.IsArray())
         {
          std::vector<TNUM> nvalues;
          for (rapidjson::SizeType cai = 0; cai < ca.Size(); cai++)
           {
            if (ca[cai].IsNumber())
             {
              TNUM val = ca[cai].Get<TNUM>();
              nvalues.push_back(val);
              newblob.initialize(start, nvalues, depth,
                                 repeat, repeat_start, repeat_end);
             }
           }
         }
       }
      else continue; // No valid blob.
#else // defined JSON_INSTEAD_OF_YAML
      YAML::Node vnam, sstart; 
      if (!(vnam = m["name"])) 
       {
        YAML::Node inam;
        if (!(inam = m["include"])) continue;
        if (!(inam.IsScalar())) continue;
        initialize(inam.as<std::string>().c_str(), head_el);
        continue;
       }
      if (!(sstart = m["start"])) continue;
      if (!(sstart.IsScalar())) continue;
      std::string snam(vnam.as<std::string>());
      qtl_config_blob<TNUM> newblob(snam);
      int depth = 0;
      bool repeat = false;
      qtl_ms_t start, end, repeat_start, repeat_end;
      start = end = repeat_start = repeat_end = 0;
      start = sstart.as<qtl_ms_t>();
      YAML::Node vm; 
      if (vm = m["depth"])
       {
        if (vm.IsScalar()) depth = vm.as<int>();
       }
      if (vm = m["end"])
       {
        if (vm.IsScalar()) end = vm.as<qtl_ms_t>();
        if (end != start) ++end; // Upper boundary is not included
                                 // in the interval. But equal values
                                 // mean forever. 
       }
      if (vm = m["repeat"])
       {
        if (vm.IsScalar()) repeat = vm.as<bool>();
       }
      if (vm = m["repeat_start"])
       {
        if (vm.IsScalar()) repeat_start = vm.as<qtl_ms_t>();
       }
      if (vm = m["repeat_end"])
       {
        if (vm.IsScalar()) repeat_end = vm.as<qtl_ms_t>();
       }
      if (vm = m["value"])
       {
        TNUM value;
        if (vm.IsScalar()) value = vm.as<TNUM>();
        newblob.initialize(start, end, value, depth); 
       }
      else if (vm = m["expr"])
       {
        std::string exs(vm.as<std::string>());
        if (vm.IsScalar()) newblob.initialize(start, end,
                             exs, depth, repeat,
                             repeat_start, repeat_end );
       }
      else if (vm = m["compose"])
       {
        if (vm.IsSequence())
         {
          for (int cai = 0; cai < vm.size(); cai++)
           {
            if (vm[cai].IsScalar())
             {
              std::string cels(vm[cai].as<std::string>());
              newblob.add_dep_blob(cels);
             }
           }
          newblob.set_data(start, depth, repeat,
                           repeat_start, repeat_end);
         }
       }
      else if (vm = m["values"])
       {
        if (vm.IsSequence())
         {
          std::vector<TNUM> nvalues = vm.as<std::vector<TNUM> >();
          newblob.initialize(start, nvalues, depth,
                             repeat, repeat_start, repeat_end);
         }
       }
      else continue; // No valid blob.
#endif // not defined JSON_INSTEAD_OF_YAML
      // Insert new new blob where it should go.
      typename qtl_blob_container_t::iterator dbl = m_blobs.find(depth);
      if (dbl == m_blobs.end())
       {
        qtl_blob_map_t newmap;
        newmap.insert(std::make_pair(newblob.get_name(), newblob));
        m_blobs.insert(std::make_pair(depth, newmap));
        ++m_count;
       }
      else
       {
        typename qtl_blob_map_t::iterator dit = dbl->second.find(newblob.get_name());
        if (dit == dbl->second.end())
         {
          dbl->second.insert(std::make_pair(newblob.get_name(),newblob));
          ++m_count;
         }
        else
         {
          dit->second.add_other_blob(newblob);
         }
       }
     }
   }

  // Resolve and compute functions by multiple iterations
  bool changed = true;
  bool all_valid;
  while (changed)
   {
    changed = false;
    all_valid = true;
    typename qtl_blob_container_t::iterator bit;
    typename qtl_blob_container_t::iterator bend = m_blobs.end();        
    for (bit = m_blobs.begin(); bit != bend; ++bit)
     {
      typename qtl_blob_map_t::iterator bn;
      typename qtl_blob_map_t::iterator bnend = bit->second.end();
      for (bn = bit->second.begin(); bn != bnend; ++bn)
       {
        if (bn->second.valid()) continue;
        if (bn->second.resolve(bit->second)) changed = true;
        else all_valid = false;
        for (int i=0; i < bn->second.other_blobs.size(); ++i)
         {
          if (bn->second.other_blobs[i].valid()) continue;
          if (bn->second.other_blobs[i].resolve(bit->second)) changed = true;
         }
       }
     }
   }
  if (!all_valid)
   {
    std::cerr << "DEBUG: Warning: not all blobs are valid." << std::endl;
   }
}

#endif /* defined _QUANTITY_TIMELINES_HPP */
