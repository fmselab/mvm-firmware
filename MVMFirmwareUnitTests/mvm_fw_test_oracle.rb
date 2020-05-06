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

require 'json'
require 'pp'
require 'mvm_fw_test_oracle_helper'

raise "Usage: #{$0} json_config" if(ARGV.length!=1)

cfile = File.read(ARGV[0])
conf = JSON.parse(cfile)
oh = Mvm_Fw_Test_Oracle_Helper.new(conf);

fkey = "SerialTTY"
lkey = "LogFile"
ckey = "CheckRubyObj"

if (!conf.key?(fkey))
  raise "#{$0}: #{fkey} not defined in config file #{ARGV[0]}"
end
 
oh.digest_file(conf[fkey])
if (conf.key?(lkey)); oh.digest_file(conf[lkey]) end

#pp oh.rhsh

rchk = oh.run_checks_in_config()

p oh.report

if (!rchk); exit 99 end

# More checks should be added here...

if (conf.key?(ckey))
  require conf[ckey]
  eval "rchk = " + conf[ckey] + ".check(oh)"
  eval "p " + conf[ckey] + ".report"
  if (!rchk); exit 98 end
end

exit 0
