#!/usr/bin/ruby
# coding: utf-8

=begin

  File :     mvm_fw_test_run_scenarios.rb

  Author :   Francesco Prelz ($Author: printmi $)
  e-mail :   "francesco.prelz@mi.infn.it"

  Revision history :
   6-May-2020 Original release

  Description: Test main for Mvm_Fw_Test_Oracle_Helper.

=end

CSHomeDir = File.expand_path(File.dirname(__FILE__))
CSScenarioDir = File.join(CSHomeDir, "..", "scenarios")
CSCheckExe = File.join(CSHomeDir, "mvm_fw_unit_test_run")

Cfkey = "SerialTTY"
Clkey = "LogFile"
Cckey = "CheckRubyObj"
Cidkey = "scenario_id"
Csdkey = "scenario_descr"
Csrkey = "scenario_ref_req"

$LOAD_PATH.unshift CSHomeDir

require 'yaml'
require 'pp'
require 'cgi'
require 'mvm_fw_test_oracle_helper'
require 'mvm_fw_test_gcov_helper'

# Traverse scenario directory tree and look for JSON files 

jsons = Dir.glob(CSScenarioDir + "/**/*.json") 

gh = Mvm_Fw_Test_Gcov_Helper.new(CSHomeDir, "MVMFirmwareCore")

tout = "<H1>MVM FW Scenario test results</H1>\n"
tout << "<H2>as of: " + Time.now.strftime("%d/%m/%Y %H:%M") + "</H2>\n"
tout << "<Table>\n<Tr><Td>Scenario Id</Td><Td>Description</Td>" +
        "<Td>Covered reqs</Td><Td>Coverage</Td><Td>Result</Td></Tr>\n"

retcode = 0

jsons.each do |jsf|

  tdir = File.dirname(jsf);
  curdir = Dir.pwd

  cfile = File.read(jsf)
  conf = YAML.load(cfile)

  if (conf.key?(Cidkey))
    newdir = File.join(tdir, conf[Cidkey].upcase)
    if (File.directory?(newdir))
      STDERR.puts "#{$0}: Directory #{newdir} exists. Skipping." 
      next
    end
    Dir.mkdir(newdir)
    Dir.chdir(newdir)
  end
  
  gh.reset()

  if (!system(CSCheckExe + " " + jsf))
    STDERR.puts "#{$0}: Error running #{CSCheckExe} on conf file #{jsf}." 
    retcode = retcode + 1
    next
  end

  oh = Mvm_Fw_Test_Oracle_Helper.new(conf);

  if (!conf.key?(Cfkey))
    STDERR.puts "#{$0}: #{Cfkey} not defined in config file #{jsf}"
    retcode = retcode + 1
    next
  end
   
  oh.digest_file(conf[Cfkey])
  if (conf.key?(Clkey)); oh.digest_file(conf[Clkey]) end
  
  #pp oh.rhsh
  
  rchk = oh.run_checks_in_config()
  trep = ""
  if (!rchk); trep = oh.report end

  # More checks should be added here...
  
  cchk = true
  if (conf.key?(Cckey))
    require conf[Cckey]
    eval "cchk = " + conf[Cckey] + ".check(oh)"
    eval "trep << \" - \" + " + conf[Cckey] + ".report"
  end

  # Gather GCOV data
  gh.evaluate()

  # Move GCOV files over
  if (gh.gcov_files)
    gh.gcov_files.each do |f|
      if (!File.rename(File.join(CSHomeDir, f), File.join(Dir.pwd, f)))
        STDERR.puts "#{$0}: error moving #{f} to cur dir " + Dir.pwd
      end
    end
  end

  tout << "<Tr><Td>"
  if (conf.key?(Cidkey))
    tout << "<B> "+ conf[Cidkey] + "</B>"
  else
    tout << "<B>UNK</B>"
  end
  tout << "</Td><Td>"
  if (conf.key?(Csdkey))
    tout << conf[Csdkey]
  else
    tout << "<I>Unknown</I>"
  end
  tout << "</Td><Td>"
  if (conf.key?(Csrkey))
    tout << conf[Csrkey]
  else
    tout << "Unknown"
  end
  tout << "</Td><Td>" + gh.checked_lines.to_s + "/" +\
                       gh.total_lines.to_s + " (" +
                       sprintf("%5.1d",gh.cperc) +
                       " &percnt;)"
  tout << "</Td><Td>"
  if (rchk && cchk)
    tout << "<B><Font color=\"green\">PASSED</Font></B>"
  else
    tout << "<B><Font color=\"red\">FAILED</Font></B>"
    tout << " (" + CGI.escapeHTML(trep) + ")"
  end
  tout << "</Td></Tr>\n"
  Dir.chdir(curdir)
end
tout << "</Table>\n"

File.open(File.join(CSScenarioDir, "README.md"), 'w') do |f|
  f.write(tout) 
end

exit retcode
