//============================================================================
// Name        : temp_testfirmware.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstring>
#include <thread>

#include "HAL.h"
#include "MVM_StateMachine.h"
#include "MVMCore.h"
using namespace std;

class MockHAL: public HAL {

};
int main() {
	// hal
	//HAL myHAL;
	// alarm
	//AlarmClass MVM_Alarms;
	//t_SystemStatus sys_s;
	//t_config core_config;
	//ConfigManagerClass  confmanager;
	MVMCore mvm;
	try {
		mvm.Init();
		for(int i= 0; i< 10; i++){
			mvm.Tick();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			cout << mvm.GetParameter("all")<< endl;
		}
		// start running
		//core_config.run = true;
		for(int i= 0; i< 10; i++){
			mvm.Tick();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			cout << mvm.GetParameter("all")<< endl;
		}
	} catch (const std::exception &e) {
		std::cerr << __FUNCTION__ << " caught unhandled exception. what(): "
				<< e.what() << std::endl;
	}

}
