// Alarms.h

#ifndef _ALARMS_h
#define _ALARMS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "HAL.h"
#include "SystemStatus.h"
#include "generic_definitions.h"


class AlarmClass
{
private:
	HAL* _HAL;
	t_SystemStatus *_sys_c;

	void CheckStaticAlarms();
	void Sound();
    void AlarmActions();
    uint32_t GenerateFlag(int alarm_code);


    uint32_t ALARM_FLAG = 0x0;
    uint32_t ALARM_FLAG_SNOOZE = 0x0;
    uint32_t ALARM_FLAG_FILTERED = 0x0;
    uint64_t ALARM_FLAG_SNOOZE_millis = 0;

    int alarm_state=0;

    uint64_t buzzer_time;
    uint64_t blinker_led_time;
    uint64_t wdog_timer;
    bool AlarmSound=false;
    bool isInAlarm = false;
    bool led_on = false;
    bool wdog_enable=false;
    float P0Loop=0;
    float P0Patient=0;
    
    CircularBuffer* CycleCyclePLoop;
    CircularBuffer* CycleCyclePPatient;

public:
	void Action_OverPressureSecurity();
	void Init(HAL* hal, t_SystemStatus* sys_c);
    void TriggerAlarm(t_ALARM Alarm);
    void ResetAlarm();
    void TransitionInhaleExhale_Event();
    void TransitionEndCycleEvent();
    void TransitionNewCycleEvent();
    void Tick();
    void ResetWatchDog();
    void EnableWatchDog(bool enable);
    void SetAlarmGUI(bool in_alarm);
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
