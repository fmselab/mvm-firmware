#define BOOST_TEST_MODULE My Test

#undef  HARDWARE_TARGET_PLATFORM_V3
#undef  HARDWARE_TARGET_PLATFORM_V4

#include <boost/test/included/unit_test.hpp>
#include "simulated_fw_board_v4.h" // Our HW abstraction
#include "Serial.h" // Our HW abstraction
#include "MVMCore.h"

// The following can go away when all leftover references disappear from
// the firmware code.
extern WireImpl Wire;
WireImpl Wire;

std::fstream IOS_ttys;
extern SerialImpl Serial;
SerialImpl Serial(IOS_ttys);

BOOST_AUTO_TEST_CASE(first_test) {
	MVMCore mvmcore;
}
