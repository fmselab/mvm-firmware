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
              :t_pressure, :t_temperature, :t_pv1,
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
  @t_pv1=nil
  @t_failed=false

  Type_regexps = { 
                   :valves_closed => Regexp.new('- *VALVES CLOSED *-'),
                   :valves_ok   => Regexp.new('- *VALVES OK *-'),
                   :out_valve   => Regexp.new('GPIO *- *DEVS *- *OUT_VALVE'),
                   :breathe     => Regexp.new('GPIO *- *DEVS *- *BREATHE'),
                   :buzzer      => Regexp.new('GPIO *- *DEVS *- *BUZZER'),
                   :alarm_led   => Regexp.new('GPIO *- *DEVS *- *ALARM_LED'),
                   :alarm_relay => Regexp.new('GPIO *- *DEVS *- *ALARM_RELAY'),
                   :pv1         => Regexp.new('GPIO *- *DEVS *- *PV1'),
                   :ps0         => Regexp.new('MS5525DSO *- *PS0.*D[12] Setup'),
                   :ps1         => Regexp.new('MS5525DSO *- *PS1.*D[12] Setup'),
                   :ps2         => Regexp.new('MS5525DSO *- *PS2.*D[12] Setup')
                 }

  def <=>(other)
    t_ms <=> other.t_ms
  end

  def initialize(line)

    if (m=/- +([[:digit:]]+):([[:digit:]]+) +-/.match(line))
      @t_abs = Time.at(m[1].to_f+m[2].to_f/1000.0)
    end
    if (m=/- +ms[^:]*:([[:digit:]]+)/.match(line))
      @t_ms = m[1].to_i
    end
    if (m=/- +tick: *([[:digit:]]+)/.match(line))
      @t_tick = m[1].to_i
    end
    if (m=/- +value set to +([[:digit:]]+)/.match(line))
      @t_value = m[1].to_i
    elsif (m=/- +current_value_is +([[:digit:]]+)/.match(line))
      t_failed = true
      @t_value = m[1].to_i
    end
    if (m=/- +PV1: *([[:digit:]]+)/.match(line))
      @t_pv1 = m[1].to_i
    end
    if (m=/temperature *== *([[:digit:]\.]+)/i.match(line))
      @t_temperature = m[1].to_f
    end
    if (m=/pressure *== *([[:digit:]\.]+)/i.match(line))
      @t_pressure = m[1].to_f
    end
    if (m=/- +alarms: *(0x)?([0-9a-fA-F]+)/.match(line))
      @t_alarms = m[2].to_i(16)
    end
    if (m=/- +warnings: *(0x)?([0-9a-fA-F]+)/.match(line))
      @t_warnings = m[2].to_i(16)
    end
    if (m=/- +command: *(0x)?([0-9a-fA-F]+)/.match(line))
      @t_command = m[2].to_i(16)
    end

    # Scan for various record types
    Type_regexps.each do |type, regex|
      m = regex.match(line)
      if (m)
        @type = type
        return
      end
    end
  end

  def failed()
    return @m_failed
  end

  def val(name)
    return nil if (!name)
    case name
      when "abstime"
        return @t_abs
      when "ms"
        return @t_ms
      when "tick"
        return @t_tick
      when "value"
        return @t_value
      when "pressure"
        return @t_pressure
      when "temperature"
        return @t_temperature
      when "pv1"
        return @t_pv1
      when "alarms"
        return @t_alarms
      when "warnings"
        return @t_warnings
      when "commands"
        return @t_command
      else
        return self[name.to_sym]
    end
  end
end

class Mvm_Fw_Test_Oracle_Helper
  
  attr_reader(:rhsh, :report)
  
  @config = nil
  @report = nil

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
            @rhsh[r.type].push(r)
          else
            @rhsh[r.type] = [r]
          end
        end
      end
    end
    @rhsh.each do |k, v|
      v.sort
    end 
  end

  def last_before_ts(type, ts)
    prev_e = nil;
    @rhsh[type].each do |e|
      return prev_e if ( e.t_ms > ts )
      prev_e = e
    end  
    return prev_e
  end

  def next_after_ts(type, ts)
    prev_e = nil;
    @rhsh[type].each do |e|
      if (e.t_ms >= ts)
        return e if ((!prev_e) || ((prev_e) && (prev_e.t_ms < ts))) 
      end
    end  
    return nil
  end

  def run_checks_in_config()
    ret = true
    if (!@config)
      @report = "No configuration file."
      ret = false
      return ret
    end
    @report = ""
    if (@rhsh.key?(:valves_closed))
      ev = @rhsh[:valves_closed]
      @report << " - Both valves found CLOSED at time: " + ev.t_ms.to_s + " ms."
      ret = false
    end
    if (tst = @config["oracle_checks"])
      tst.each do |e| 
        next if (!e.key?("event"))
        evs = e["event"].to_sym
        if (!evs)
          ret = false
          @report << " - unknown event <" + e["event"] + ">"
          next
        end
        if (!@rhsh.key?(evs))
          ret = false
          @report << " - no activity found for event <" + e["event"] + ">"
          next
        end
        pev = nil
        if (e.key?("after"))
          after_ts  = e["after"]
          pev = next_after_ts(evs, after_ts)
          if (!pev)
            ret = false
            @report << " - no event for <" + evs.to_s + 
                       "> after t==" + after_ts.to_s
            next
          end
          if (e.key?("before"))
            before_ts = e["before"] 
            if (before_ts && pev.t_ms > before_ts)
              ret = false
              @report << " - no event for <" + evs.to_s +
                         "> after t==" + after_ts.to_s +
                         " and before t==" + before_ts.to_s
              next
            end
          end
        elsif (e.key?("before"))
          before_ts = e["before"] 
          pev = last_before_ts(evs, before_ts)
          if (!pev)
            ret = false
            @report << " - no event for <" + evs.to_s + 
                       "> before t==" + before_ts.to_s
            next
          end
        end
        if (pev)
          reqs = e["reqs"] 
          reqs.each do |r|
            if (!(v = pev.val(r["attr"])))
              ret = false
              @report << " - attribute " + r["attr"] + " not found in <" +
                         evs.to_s + "> event at t==" + pev.t_ms.to_s
              next
            end
            if (r.key?("eq"))
              eq = r["eq"]
              if (!r.key?("tolerance"))
                if (v != eq)
                  ret = false
                  @report << " - attribute " + r["attr"] + " in <" +
                         evs.to_s + "> event at t==" + pev.t_ms.to_s +
                         " != " + eq.to_s + " ( == " + v.to_s + " )"
                end
              else
                tol = r["tolerance"]
                if ((v < (eq - tol)) || (v > (eq + tol)))
                  ret = false
                  @report << " - attribute " + r["attr"] + " in <" +
                         evs.to_s + "> event at t==" + pev.t_ms.to_s +
                         " != " + eq.to_s + " +/- " + tol.to_s +
                         " ( == " + v.to_s + " )"
                end
              end 
            end
            if (r.key?("le"))
              le = r["le"]
              if (v > le)
                ret = false
                @report << " - attribute " + r["attr"] + " in <" +
                       evs.to_s + "> event at t==" + pev.t_ms.to_s +
                       " > " + le.to_s + " ( == " + v.to_s + " )"
              end
            end
            if (r.key?("ge"))
              ge = r["ge"]
              if (v < ge)
                ret = false
                @report << " - attribute " + r["attr"] + " in <" +
                       evs.to_s + "> event at t==" + pev.t_ms.to_s +
                       " < " + ge.to_s + " ( == " + v.to_s + " )"
              end
            end
            if (r.key?("andon"))
              andv = r["andon"]
              if (!(v & andv))
                ret = false
                @report << " - attribute " + r["attr"] + " in <" +
                       evs.to_s + "> event at t==" + pev.t_ms.to_s +
                       " & " + andv.to_s + " OFF ( == " + v.to_s + " )"
              end
            end
            if (r.key?("andoff"))
              andv = r["andoff"]
              if (v & andv)
                ret = false
                @report << " - attribute " + r["attr"] + " in <" +
                       evs.to_s + "> event at t==" + pev.t_ms.to_s +
                       " & " + andv.to_s + " ON ( == " + v.to_s + " )"
              end
            end
          end
        end
      end
    end
    if (!ret) 
      @report << " - Test FAILED."
    else
      @report << " - All tests succeeded."
    end
    return ret
  end

end
