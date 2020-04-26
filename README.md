# mvm-firmware

mvm-firmware: the tests for thh firware for MVM.

In this repo there is NOT the last versione of the firmware for the MVM. 
We put here only the tests and the scaffolding  (stubs, drivers, ...) for them.

the directories:

* MVMFirmwareCpp contains the code original from Andrea's repo as submodule
* MVMFirmwareUnitTests Francesco's code for runnign the tests with mock for devices (json)
* test_hwlevel: Angelo's attempt to run the tests with Francesco's code + boost unit 
* test_hal_level: Angelo's attempt ro run the tests with a mock ar HAL level
