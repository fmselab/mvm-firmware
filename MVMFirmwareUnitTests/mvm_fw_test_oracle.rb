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

if (!conf.key?(fkey))
  raise "#{$0}: #{fkey} not defined in config file #{ARGV[0]}"
end
 
oh.digest_file(conf[fkey])
oh.digest_file(conf[lkey]) if (conf.key?(lkey))

#pp oh.rhsh

rchk = oh.run_checks_in_config()

p oh.report

exit 99 if (!rchk)

# More checks should be added here...

exit 0
