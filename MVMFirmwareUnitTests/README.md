Test harness for the [MVM](http://mvm.care) "firmware". Starting from waveforms and timelines described in a JSON configuration file (the syntax still has to be documented yet, but see [here](test_file_patient.json) for an example -may move to YAML eventually) responses for all I2C- and GPIO-connected sensors and devices are generated Failure, alarm and error conditions can be generated as well.

Running 'make' here builds (on the machine native architecture - building and executing this on some emulated
XTENSA machine could be a next steo) the mvm_fw_unit_test_run exectuable, whose command-line options are:

    mvm_fw_unit_test_run [-p] [-d debug level] <JSON config file>

At least (debug_level can actually be specified on the command line as well) following attributes have to be defined in the config file:

     "SerialTTY": "/path/to/serial/output/file.out",
     "LogFile:    "/path/to/another/logfile"
     "debug_level" : 5
     "end_ms": 10000,

These will redirect the MVM serial line TTY output output to the given file.

The '-p' option will cause the creation (on POSIX systems) of a new pseudoterminal in 'master' mode, that can be used to connect the MVM GUI directly:

     $./mvm_fw_unit_test_run -p test_gui_patient.json
     ./mvm_fw_unit_test_run: Successfully connected to PTS /dev/pts/17. Waiting for 10 seconds.
    
The serial dump test report and log files can also be parsed to look for error conditions and inconsistencies, as directed in the "oracle_checks" section of the JSON configuration file, and via the class contained in [mvm_fw_test_oracle_helper.rb](mvm_fw_test_oracle_helper.rb). Just run, in the same directory as the test exe:

     $ ./mvm_fw_test_oracle.rb <JSON config file>
     " - All tests succeeded."
     $ echo $?
     0

All JSON scenarios posted and present in the [mvm-firmware/scenarios](https://github.com/fmselab/mvm-firmware/tree/master/scenarios) directory tree will be automatically run by [Travis CI](https://travis-ci.com/github/fmselab/mvm-firmware), with a brief report with test tesylts and code coverage being automatically pushed back to GitHub [Here](https://github.com/fmselab/mvm-firmware/blob/master/scenarios/README.md).