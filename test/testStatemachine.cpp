#define BOOST_TEST_MODULE My Test

//#undef  HARDWARE_TARGET_PLATFORM_V3
//#undef  HARDWARE_TARGET_PLATFORM_V4

#include <boost/test/included/unit_test.hpp>
#include "simulated_fw_board_v4.h" // Our HW abstraction
#include "Serial.h" // Our HW abstraction
#include "MVMCore.h"
#include <iostream>

// The following can go away when all leftover references disappear from
// the firmware code.
extern WireImpl Wire;
WireImpl Wire;

std::fstream IOS_ttys;
extern SerialImpl Serial;
SerialImpl Serial(IOS_ttys);
mvm_fw_unit_test_config FW_TEST_main_config;
quantity_timelines<double> FW_TEST_qtl_double;
qtl_tick_t FW_TEST_tick;

BOOST_AUTO_TEST_CASE(first_test) {
	MVMCore mvm;
	mvm.Init();
	//
	BOOST_CHECK_EQUAL("",mvm.GetParameter("bpm"));
	mvm.Tick();
}
