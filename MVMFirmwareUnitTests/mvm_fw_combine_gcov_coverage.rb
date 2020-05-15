# coding: utf-8

=begin

  File :     mvm_fw_combine_gcov_coverage.rb

  Author :   Francesco Prelz ($Author: printmi $)
  e-mail :   "francesco.prelz@mi.infn.it"

  Revision history :
  13-May-2020 Original release

  Description: Compute total execution coverage from a bunch of GCOV files
               collected from separate executions.

=end

class Mvm_Fw_Combine_Gcov_Coverage

  attr_reader(:total_lines, :checked_lines, :cperc, :report)

  @total_lines = 0
  @checked_lines = 0
  @cperc = 0
  @report = ""

  def initialize(gcoll)
    @fcounted = {}
    @fmiss = {}
    @total_lines = 0
    @missed_lines = 0
    gcoll.each do |gcf|
      gcf.each do |cf|
        fkey = File.basename(cf)
        File::open(cf,'r') do |f|
          f.each_line do |l|
            ul = l.encode("UTF-8", :invalid => :replace,
                                   :undef => :replace,
                                   :replace => "")
            if (m=/^\s*[0-9\*]+:\s*([0-9]+)\s*:/.match(ul))
              line = m[1].to_i
              if (!(@fcounted.key?(fkey)))
                @total_lines = @total_lines + 1
              elsif ((@fmiss.key?(fkey)) &&
                     (@fmiss[fkey].key?(line)) &&
                     (@fmiss[fkey][line]))
                @missed_lines = @missed_lines - 1
                @fmiss[fkey].delete(line)
              end
            elsif (m=/^\s*#+:\s*([0-9]+)\s*:/.match(ul))
              line = m[1].to_i
              if (!(@fcounted.key?(fkey)))
                @total_lines = @total_lines + 1
                @missed_lines = @missed_lines + 1
                if (!@fmiss.key?(fkey)); @fmiss[fkey] = {} end
                @fmiss[fkey][line] = true
              end
            end
          end
        end
        @fcounted[fkey] = true
      end
    end
    @checked_lines = @total_lines - @missed_lines
    @cperc = 0
    if (@total_lines > 0)
      @cperc = (@checked_lines.to_f / @total_lines.to_f) * 100
    end
    @report = "TOTAL coverage: " + @checked_lines.to_s + "/" + @total_lines.to_s +
               sprintf(" - %5.1f%%", @cperc)

  end

end
