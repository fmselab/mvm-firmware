#!/usr/bin/ruby
# coding: utf-8

=begin

  File :     mvm_fw_test_oracle.rb

  Author :   Francesco Prelz ($Author: printmi $)
  e-mail :   "francesco.prelz@mi.infn.it"

  Revision history :
   5-May-2020 Original release

  Description: Test main for Mvm_Fw_Test_Oracle_Helper.

=end

$LOAD_PATH.unshift File.dirname(__FILE__)

require 'yaml'
require 'pp'
require 'mvm_fw_test_oracle_helper'

raise "Usage: #{$0} config (json|yml)" if(ARGV.length!=1)

rfile = ARGV[0]
cfile = File.read(rfile)
conf = YAML.load(cfile)
oh = Mvm_Fw_Test_Oracle_Helper.new(conf, File.dirname(rfile));

fkey = "SerialTTY"
lkey = "LogFile"
ckey = "CheckRubyObj"

ff = oh.key(fkey)
if (!ff)
  raise "#{$0}: #{fkey} not defined in config file #{ARGV[0]}"
end
 
oh.digest_file(ff)
lf = oh.key(lkey)
if (lf); oh.digest_file(lf) end

#pp oh.rhsh

rchk = oh.run_checks_in_config()

p oh.report

if (!rchk); exit 99 end

# More checks should be added here...

lc = oh.key(ckey)
if (lc)
  require lc
  eval "rchk = " + lc + ".check(oh)"
  eval "p " + lc + ".report"
  if (!rchk); exit 98 end
end

exit 0
