#undef  HARDWARE_TARGET_PLATFORM_V3
#undef  HARDWARE_TARGET_PLATFORM_V4

#include "MVMCore.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "scaledtime.h"
#include "catch.hpp"

using namespace std;

enum state {INHALE, EXHALE};

//TEST_CASE( "MVM_set_parameters_test()","MVM") {
//	MVMCore mvm;
//	// set a right parameter
//	REQUIRE(mvm.SetParameter("rate","6"));// "rate set");
//	REQUIRE(mvm.GetParameter("rate") == "6");
//	// set wrong parameters
//	REQUIRE(! mvm.SetParameter("notexisting","6"));//,"setting not exiting parameter should return false");
//	REQUIRE(mvm.GetParameter("rate")== "6"); // no change
//	REQUIRE(! mvm.SetParameter("rate","invalidvalue"));//,"setting not exiting parameter should return false");
//	REQUIRE(mvm.GetParameter("rate") == "6"); // no change
//}


TEST_CASE( "MVM_normal_scenario","MVM"){

	MVMCore mvm;
	cout << "init the machine" << std::endl;
	mvm.Init();
	std::cout << mvm.GetParameter("all").c_str() << std::endl;

	mvm.SetParameter("rate","5"); // 12 seconds each
	mvm.SetParameter("ratio","0.5");//I:E=1:2: I: 4 E: 8
	mvm.SetParameter("run", "1");
	long startcycle = getScaledMillisecfromInit();

	//BOOST_CHECK_EQUAL("",mvm.GetParameter("bpm"));
	cout << "pPatient, Flux" << ",last_O2" << ",last_bpm" << ",TidalVolume"
			<< "last_peep" << "GasTemperature" << ",batteryPowered"
			<< "currentBatteryCharge" << "currentP_Peak"
			<< "currentTvIsnp*1000.0" << ",currentTvEsp * 1000.0"
			<< "currentVM"<< std::endl;
	state supposedState = INHALE;
	state lastState = supposedState;
	for (int i = 0; i <= 100; i++) {
		mvm.Tick();
		// every 12 seconds a new cycle starts
		long timefromStartCycle = (startcycle - getScaledMillisecfromInit()) % (12 * 1000);
		// the first 4 secs
		state supposedState = ( 0 <= timefromStartCycle && timefromStartCycle < 4 * 1000) ? INHALE : EXHALE;
		if (lastState != supposedState) {
			cout<< supposedState;
			lastState = supposedState;
		}
		// wait 0.2 seconds
		scaledSleep(200);
	}
}
