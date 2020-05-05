# coding: utf-8

=begin

  File :     mvm_fw_test_oracle_helper.rb

  Author :   Francesco Prelz ($Author: printmi $)
  e-mail :   "francesco.prelz@mi.infn.it"

  Revision history :
   4-May-2020 Original release

  Description: Simple ruby object meant to digest a MVM firmware serial line
               dump/debug log and look for interesting events

=end

require 'json'

class Mvm_Fw_Test_Log_Event
  attr_reader(:type, :t_abs, :t_ms, :t_tick, :t_value,
              :t_pressure, :t_temperature,
              :t_alarms, :t_warnings, :t_command)

  @type=nil
  @t_abs=nil
  @t_ms=nil
  @t_tick=nil
  @t_value=nil
  @t_pressure=nil
  @t_temperature=nil
  @t_alarms=nil
  @t_warnings=nil
  @t_command=nil

  Type_regexps = { :out_valve  => Regexp.new('GPIO *- *DEVS *- *OUT_VALVE'),
                   :ps0        => Regexp.new('MS5525DSO *- *PS0.*D[12] Setup'),
                   :ps1        => Regexp.new('MS5525DSO *- *PS1.*D[12] Setup'),
                   :ps2        => Regexp.new('MS5525DSO *- *PS2.*D[12] Setup') }

  def initialize(line)

    if (m=/- +([[:digit:]]+):([[:digit:]]+) +-/.match(line))
      @t_abs = Time.at(m[1].to_f+m[2].to_f/1000.0)
    end
    if (m=/- +ms[^:]*:([[:digit:]]+)/.match(line))
      @t_ms = m[1].to_i;
    end
    if (m=/- +tick: *([[:digit:]]+)/.match(line))
      @t_tick = m[1].to_i;
    end
    if (m=/- +value set to +([[:digit:]]+)/.match(line))
      @t_value = m[1].to_i;
    end
    if (m=/temperature *== *([[:digit:]\.]+)/i.match(line))
      @t_temperature = m[1].to_f;
    end
    if (m=/pressute *== *([[:digit:]\.]+)/i.match(line))
      @t_temperature = m[1].to_f;
    end
    if (m=/- +alarms: *(0x)?([0-9a-fA-F]+)/.match(line))
      @t_alarms = m[2].to_i(16);
    end
    if (m=/- +warnings: *(0x)?([0-9a-fA-F]+)/.match(line))
      @t_warnings = m[2].to_i(16);
    end
    if (m=/- +command: *(0x)?([0-9a-fA-F]+)/.match(line))
      @t_command = m[2].to_i(16);
    end

    # Scan for various record types
    Type_regexps.each do |type, regex|
      m = regex.match(line);
      if (m)
        @type = type
        return
      end
    end
  end
end

class Mvm_Fw_Test_Oracle_Helper
  
  attr_reader(:rhsh)
  
  def initialize(conf)
    @config = conf
    @rhsh = {}
  end
  
  def digest_file(filename)
    File::open(filename,'r') do |f|
      pp f
      f.each_line do |l|
        r=Mvm_Fw_Test_Log_Event.new(l.chomp)
        if (r.type)
          if (@rhsh.key?(r.type))
            rhsh[r.type].push(r)
          else
            @rhsh[r.type] = [r]
          end
        end
      end
    end
  end
end
