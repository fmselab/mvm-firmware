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
using namespace std;

class MockHAL: public HAL {

};
int dddd() {
	MVM_StateMachine machine;
	// hal
	HAL myHAL;
	myHAL.Init();
	// alarm
	AlarmClass MVM_Alarms;
	t_SystemStatus sys_s;
	t_config core_config;
	try {
		MVM_Alarms.Init(&myHAL, &sys_s);
		machine.Init(&myHAL, &MVM_Alarms, &core_config, &sys_s, 10);
		for(int i= 0; i< 10; i++){
			machine.Tick();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			cout << machine.dbg_state_machine << endl;
		}
		// start running
		core_config.run = true;
		for(int i= 0; i< 10; i++){
			machine.Tick();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			cout << machine.dbg_state_machine << endl;
		}
	} catch (const std::exception &e) {
		std::cerr << __FUNCTION__ << " caught unhandled exception. what(): "
				<< e.what() << std::endl;
	}

}
