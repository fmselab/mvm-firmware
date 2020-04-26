//============================================================================
// Name        : temp_testfirmware.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstring>

#include "mock/HAL.h"
#include "MVM_StateMachine.h"
using namespace std;

class MockHAL: public HAL{

};
int main() {
	MVM_StateMachine machine;
	// hal
	HAL myHAL;
	myHAL.Init();
	// alarm
	AlarmClass *_MVM_Alarms;
	t_config *core_config;
	t_SystemStatus *sys_c;
	int32_t _dT = 10;
	// config
	machine.Init(&myHAL, _MVM_Alarms, core_config, sys_c, _dT);
	machine.Tick();
	//
}
