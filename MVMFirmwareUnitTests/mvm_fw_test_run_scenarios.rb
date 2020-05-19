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
require 'mvm_fw_combine_gcov_coverage'

# Traverse scenario directory tree and look for JSON files 

jsons = Dir.glob(CSScenarioDir + "/**/*.json") 
yamls = Dir.glob(CSScenarioDir + "/**/*.yml") 
confs = jsons + yamls

gh = Mvm_Fw_Test_Gcov_Helper.new(CSHomeDir, "MVMFirmwareCore")

tout = "<H1>MVM FW Scenario test results</H1>\n"
tout << "<H2>as of: " + Time.now.strftime("%d/%m/%Y %H:%M") + "</H2>\n"
tout << "<Table>\n<Tr><Td>Scenario Id</Td><Td>Description</Td>" +
        "<Td>Covered reqs</Td><Td>Coverage</Td><Td>Result</Td></Tr>\n"

retcode = 0
gcov_coll = []

confs.each do |jsf|

  tdir = File.dirname(jsf);
  curdir = Dir.pwd

  cfile = File.read(jsf)
  conf = YAML.load(cfile)

  oh = Mvm_Fw_Test_Oracle_Helper.new(conf, tdir);

  id = oh.key(Cidkey)

  need_to_run = true
  checkeds = ""
  totals   = ""
  cperc    = 0

  if (id)
    newdir = File.join(tdir, id.upcase)
    if (File.directory?(newdir))
      # Retrieve gcov files for final report
      ecovs = (Dir.glob(newdir + "/*.gcov")) 
      STDERR.puts "#{$0}: Directory #{newdir} exists. Skipping run." 
      ogc = Mvm_Fw_Combine_Gcov_Coverage.new([ ecovs ])
      checkeds = ogc.checked_lines.to_s
      totals   = ogc.total_lines.to_s
      cperc    = ogc.cperc
      need_to_run = false
    else
      Dir.mkdir(newdir)
      Dir.chdir(newdir)
    end
  end
  
  if (need_to_run)
    gh.reset()
  
    if (!system(CSCheckExe + " " + jsf))
      STDERR.puts "#{$0}: Error running #{CSCheckExe} on conf file #{jsf}." 
      retcode = retcode + 1
      next
    end

    # Gather GCOV data
    gh.evaluate()
    checkeds = gh.checked_lines.to_s
    totals   = gh.total_lines.to_s
    cperc    = gh.cperc
    # Move GCOV files over
    dest_gcov = []
    if (gh.gcov_files)
      gh.gcov_files.each do |f|
        dfile = File.join(Dir.pwd, f)
        if (!File.rename(File.join(CSHomeDir, f), dfile))
          STDERR.puts "#{$0}: error moving #{f} to cur dir " + Dir.pwd
        else
          dest_gcov.push(dfile)
        end
      end
    end
    gcov_coll.push(dest_gcov)
  end

  fk = oh.key(Cfkey)
  if (!fk)
    STDERR.puts "#{$0}: #{Cfkey} not defined in config file #{jsf}"
    retcode = retcode + 1
    next
  else
    oh.digest_file(fk)
  end
   
  lk = oh.key(Clkey)
  if (lk); oh.digest_file(lk) end
  
  #pp oh.rhsh
  
  rchk = oh.run_checks_in_config()
  trep = ""
  if (!rchk); trep = oh.report end

  # More checks should be added here...
  
  cchk = true
  lc = oh.key(Cckey)
  if (lc)
    require lc
    eval "cchk = " + lc + ".check(oh)"
    eval "trep << \" - \" + " + lc + ".report"
  end


  tout << "<Tr><Td>"
  if (id)
    tout << "<B> "+ id + "</B>"
  else
    tout << "<B>UNK</B>"
  end
  tout << "</Td><Td>"
  sk = oh.key(Csdkey)
  if (sk)
    tout << sk
  else
    tout << "<I>Unknown</I>"
  end
  tout << "</Td><Td>"
  sr = oh.key(Csrkey)
  if (sr)
    tout << sr
  else
    tout << "Unknown"
  end
  tout << "</Td><Td>" + checkeds + "/" +\
                        totals + " (" +
                        sprintf("%5.1d",cperc) +
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

# Add global coverage
gc = Mvm_Fw_Combine_Gcov_Coverage.new(gcov_coll);
tout << "<Tr><Td Colspan=3>TOTAL code coverage:</Td><Td>" + gc.report + 
       "</Td><Td></Td></Tr>\n"

tout << "</Table>\n"

File.open(File.join(CSScenarioDir, "README.md"), 'w') do |f|
  f.write(tout) 
end

exit retcode
