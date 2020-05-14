# coding: utf-8

=begin

  File :     mvm_fw_test_gcov_helper.rb

  Author :   Francesco Prelz 
  e-mail :   francesco.prelz@mi.infn.it

  Revision history :
   6-May-2020 Original release

  Description: Digest gcov output and produce an overall coverage
               report.

=end

class Mvm_Fw_Test_Gcov_Helper
  
  attr_reader(:chsh, :cperc, :checked_lines, :total_lines,
              :gcov_files, :report)
  
  @total_lines = 0
  @checked_lines = 0
  @gcov_files = []
  @chsh = {}
  @cperc = 0
  @report = nil
  @target_dir = nil;
  @proj_str = nil

  def initialize(dir, csubstr)
    @curdir = Dir.pwd
    @target_dir = dir
    @proj_str = csubstr
  end

  def evaluate()
    @curdir = Dir.pwd
    if ((@target_dir) && (@target_dir.length > 0))
      Dir.chdir(@target_dir)
    end

    @checked_lines = 0
    @total_lines = 0
    @chsh = {}
    @gcov_files = []

    # Example gcov output format:
    # File '/usr/include/c++/8/tuple'
    # Lines executed:100.00% of 36
    # Creating 'tuple.gcov'

    dfd = File::open(File.join(@curdir, @proj_str + "-gcov-debug"),'w')

    cur_file = nil
    IO::popen("make gcov",'r',:err => File::NULL) do |f|
      f.each_line do |l|
        dfd.write(l)
        if (m=/File +'([^']+)'/.match(l))
          if ((@proj_str) && (@proj_str.length > 0) &&
              (!m[1].include? @proj_str))
            cur_file = nil
            next
          end
          tfile = File.basename(m[1])
          cur_file = tfile
        elsif (m=/Lines executed: *([0-9\.]+)% +of +([0-9]+)/.match(l))
          if (cur_file)
            cur_frac = m[1].to_f / 100
            cur_lines = m[2].to_f
            chk_lines = (cur_lines * cur_frac).round 
            @total_lines += cur_lines.to_i
            @checked_lines += chk_lines
            @chsh[cur_file] = { :lines => cur_lines, :checked => chk_lines }
          end
        elsif (m=/Creating +'([^']+)'/.match(l))
          if (cur_file)
            @gcov_files.push(m[1])
          end
        end
      end
    end

    dfd.close

    @cperc = 0
    if (@total_lines > 0)
      @cperc = (@checked_lines.to_f / @total_lines.to_f) * 100
    end

    if (@proj_str != nil)
      Dir.chdir(@curdir)
      @report = @proj_str + ": "
    else
      @report = ""
    end

    @report << "coverage: " +
               @checked_lines.to_s + "/" + @total_lines.to_s +
               sprintf(" - %5.1f%%", @cperc)
    if ((@target_dir) && (@target_dir.length > 0))
      Dir.chdir(@curdir)
    end
  end

  def reset()
    @curdir = Dir.pwd
    if ((@target_dir) && (@target_dir.length > 0))
      Dir.chdir(@target_dir)
    end
    system("make clean_gcov", :err => File::NULL, :out => File::NULL)
    @checked_lines = 0
    @total_lines = 0
    @chsh = {}
    if ((@target_dir) && (@target_dir.length > 0))
      Dir.chdir(@curdir)
    end
  end
end
