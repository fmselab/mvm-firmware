# coding: utf-8

=begin

  File :     mvm_fw_test_oracle_helper.rb

  Author :   Francesco Prelz 
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
              :t_alarms, :t_warnings, :t_command,
              :t_command_args, :t_command_status, :t_set, :t_failed,
              :g_ppatient, :g_flux, :g_o2, :g_bpm, :g_tidal,
              :g_peep, :g_batterypowered,
              :g_batterycharge, :g_ppeak, :g_tv_insp, :g_tv_esp,
              :g_currentvm, :g_flow, :g_ptarget, :g_psupport, :g_run)

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
  @t_command_args=nil
  @t_command_status=nil
  @t_pv1=nil
  @t_set=false
  @t_failed=false

  @g_ppatient=nil
  @g_flux=nil
  @g_o2=nil
  @g_bpm=nil
  @g_tidal=nil
  @g_peep=nil
  @g_batterypowered=nil
  @g_batterycharge=nil
  @g_ppeak=nil
  @g_tv_insp=nil
  @g_tv_esp =nil
  @g_currentvm=nil
  @g_flow=nil
  @g_ptarget=nil
  @g_psupport=nil
  @g_run=nil

  Type_regexps = { 
                   :command => Regexp.new('MAIN PROGRAM *- *SENDING COMMAND *-'),
                   :valves_closed => Regexp.new('- *VALVES CLOSED *-'),
                   :valves_ok   => Regexp.new('- *VALVES OK *-'),
                   :out_valve   => Regexp.new('GPIO *- *DEVS *- *OUT_VALVE'),
                   :breathe     => Regexp.new('GPIO *- *DEVS *- *BREATHE'),
                   :buzzer      => Regexp.new('GPIO *- *DEVS *- *BUZZER'),
                   :alarm_led   => Regexp.new('GPIO *- *DEVS *- *ALARM_LED'),
                   :alarm_relay => Regexp.new('GPIO *- *DEVS *- *ALARM_RELAY'),
                   :pv1         => Regexp.new('GPIO *- *DEVS *- *PV1'),
                   :pi3         => Regexp.new('MS5525DSO *- *PI3.*D[12] Setup'),
                   :pi2         => Regexp.new('MS5525DSO *- *PI2.*D[12] Setup'),
                   :pi1         => Regexp.new('MS5525DSO *- *PI1.*D[12] Setup')
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
    if (m=/- *SENDING COMMAND *- *(set|get) +([^ ]+)([^-]*)-/i.match(line))
      if (m[1].casecmp("set") == 0)
        @t_set = true
      else
        @t_set = false
      end
      @t_command = m[2].downcase
      @t_command_args = m[3].strip
    elsif (m=/- +command: *(0x)?([0-9a-fA-F]+)/.match(line))
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
      when "failed"
        return @t_failed
      when "commands"
        return @t_command
      else
        return self.send name.to_sym
    end
  end

  def update_command_status(line)
    if (@t_set)
      @t_command_status = line
    else
      case @t_command
        when "all"
          vals = line.split(',')
          if (vals.size > 0); @g_ppatient       = vals[0].to_f end
          if (vals.size > 1); @g_flux           = vals[1].to_f end
          if (vals.size > 2); @g_o2             = vals[2].to_f end
          if (vals.size > 3); @g_bpm            = vals[3].to_f end
          if (vals.size > 4); @g_tidal          = vals[4].to_f end
          if (vals.size > 5); @g_peep           = vals[5].to_f end
          if (vals.size > 6); @t_temperature    = vals[6].to_f end
          if (vals.size > 7); @g_batterypowered = vals[7].to_i end
          if (vals.size > 8); @g_batterycharge  = vals[8].to_f end
          if (vals.size > 9); @g_ppeak          = vals[9].to_f end
          if (vals.size > 10); @g_tv_insp       = vals[10].to_f end
          if (vals.size > 11); @g_tv_esp        = vals[11].to_f end
          if (vals.size > 12); @g_currentvm     = vals[12].to_f end
        when "ppressure"
          @g_ppatient       = line.to_f  
        when "flow"
          @g_flow           = line.to_f
        when "o2"
          @g_o2             = line.to_f
        when "bpm"
          @g_bpm            = line.to_f
        when "tidal"
          @g_tidal          = line.to_f
        when "peep"
          @g_peep           = line.to_f
        when "temperature"
          @t_temperature    = line.to_f
        when "power_mode"
          @g_batterypowered = line.to_i
        when "battery"
          @g_batterycharge  = line.to_f
        when "ptarget"
          @g_ptarget        = line.to_f
        when "pressure_support"
          @g_psupport       = line.to_f
        when "alarm"
          @t_alarm          = line.to_i
        when "warning"
          @t_warning        = line.to_i
        when "run"
          @g_run            = line.to_i
      end
    end
    if (line.downcase.include? "error")
      @t_failed = true
    else
      @t_failed = false
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
      f.each_line do |l|
        # Detect command replies
        if ((m=/valore=/.match(l)) && (@rhsh.key?(:command)))
          @rhsh[:command].sort
          @rhsh[:command].last.update_command_status(m.post_match.chomp)
        else
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
    end
    @rhsh.each do |k, v|
      v.sort
    end 
  end

  def max_between_ts(type, attr, start, bend)
    maxv = nil;
    ret = nil
    @rhsh[type].each do |e|
      if ((start != nil) && (e.t_ms < start)); last end
      if ((bend != nil)   && (e.t_ms > bend)); last end
      val = e.val(attr)
      if (val == nil); next end
      if (maxv == nil)
        maxv = val
        ret = e
      elsif (val > maxv)
        maxv = val
        ret = e
      end
    end  
    return ret
  end

  def min_between_ts(type, attr, start, bend)
    minv = nil;
    ret = nil
    @rhsh[type].each do |e|
      if ((start != nil) && (e.t_ms < start)); last end
      if ((bend != nil)   && (e.t_ms > bend)); last end
      val = e.val(attr)
      if (val == nil); next end
      if (minv == nil)
        minv = val
        ret = e
      elsif (val < minv)
        minv = val
        ret = e
      end
    end  
    return ret
  end

  def last_between_ts(type, start, bend)
    prev_e = nil;
    @rhsh[type].each do |e|
      if ((bend != nil) && (e.t_ms > bend)); return prev_e end
      if ((start == nil) || (e.t_ms >= start)); prev_e = e end
    end  
    return prev_e
  end

  def first_between_ts(type, start, bend)
    prev_e = nil;
    @rhsh[type].each do |e|
      if ((start == nil) || (e.t_ms >= start))
        if ((bend != nil) && (e.t_ms > bend)); last end
        if ((!prev_e) || ((prev_e) && (prev_e.t_ms < start))); return e end
        prev_e = e
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
        want_never = false
        if (e.key?("never"))
          want_never = e["never"]
        end
        if (!evs)
          if (!want_never)
            ret = false
            @report << " - unknown event <" + e["event"] + ">"
          end
          next
        end
        if (!@rhsh.key?(evs))
          if (!want_never)
            ret = false
            @report << " - no activity found for event <" + e["event"] + ">"
          end
          next
        end
        pev = nil
        max = nil
        min = nil
        if (e.key?("max"))
          max = e["max"]
        end
        if (e.key?("min"))
          min = e["max"]
        end
        before_ts = nil
        if (e.key?("before")); before_ts = e["before"] end
        after_ts = nil
        if (e.key?("after"));  after_ts  = e["after"] end
        if (max)
          pev = max_between_ts(evs, max, after_ts, before_ts)
        elsif (min)
          pev = min_between_ts(evs, min, after_ts, before_ts)
        else
          if (after_ts == nil) 
            pev = last_between_ts(evs, after_ts, before_ts)
          else
            pev = first_between_ts(evs, after_ts, before_ts)
          end
        end
        if (!pev)
          if (!want_never)
            ret = false
            @report << " - no event for <" + evs.to_s + 
                       "> after t==" + after_ts.to_s
          end
          next
        end
        if ((pev) && (want_never))
          ret = false
          @report << " - found unwanted event for <" + evs.to_s + 
                     "> after t==" + after_ts.to_s + ", at t==" +
                     pev.t_ms.to_s
          next
        end
        if (pev)
          if (!e.key?("reqs")); next end
          reqs = e["reqs"] 
          reqs.each do |r|
            if ((v = pev.val(r["attr"])) == nil)
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
