// MVM_StateMachine.h

#ifndef _MVM_STATEMACHINE_h
#define _MVM_STATEMACHINE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <stdint.h>


#include "ConfigManager.h"
#include "HAL.h"


typedef enum {
	FR_OPEN_INVALVE,
	FR_WAIT_INHALE_TIME,
	FR_WAIT_EXHALE_TIME,
	AST_WAIT_MIN_INHALE_TIME,
	AST_WAIT_FLUX_DROP,
	AST_WAIT_FLUX_DROP_b,
	AST_DEADTIME,
	AST_PAUSE_EXHALE,
	AST_PAUSE_EXHALEb
} t_mvm_sm;

class MVM_StateMachine
{
public:
	void Init(HAL* _MVM_HAL, AlarmClass *_MVM_Alarms, t_config* core_config, t_SystemStatus* sys_c, int32_t _dT);
	void Tick();
	uint32_t dbg_state_machine;
	
	std::function<void()> callback_NewCycle = NULL;
	std::function<void()> callback_Exhale = NULL;
	std::function<void()> callback_EndCycle = NULL;
	
	void addHandler_NewCycle(std::function<void()> callback)
	{
		callback_NewCycle = callback;
	}

	void addHandler_Exhale(std::function<void()> callback)
	{
		callback_Exhale = callback;
	}

	void addHandler_EndCycle(std::function<void()> callback)
	{
		callback_EndCycle = callback;
	}

private:
	void SMExecute();
	float timer1;
	int32_t ddT;
	float dT;
	uint64_t cycle_SMTick;
	HAL* MVM_HAL;
	t_config* core_config;
	AlarmClass* MVM_Alarms;
	t_SystemStatus* sys_c;
	t_mvm_sm mvm_sm;

	float last_isp_time;
	int timer_divider = 0;
	uint32_t last_start = 0;

};

#endif

//                  #     # ### 
//                  ##    #  #  
//                  # #   #  #  
//                  #  #  #  #  
//                  #   # #  #  
//                  #    ##  #  
//                  #     # ### 
//
// Nuclear Instruments 2020 - All rights reserved
// Any commercial use of this code is forbidden
// Contact info@nuclearinstruments.eu
