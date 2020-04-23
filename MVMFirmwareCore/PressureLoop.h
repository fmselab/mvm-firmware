// PressureLoop.h

#ifndef _PRESSURELOOP_h
#define _PRESSURELOOP_h
#include "generic_definitions.h"
#include "hw.h"
#include "DebugIface.h"
#include "DriverContext.h"

class PressureLoopClass
{
 protected:


 public:
    void Init(float fast_ms, int32_t LoopRatio, void *handle);

    void Tick();
    void PID_SLOW_LOOP();
    void PID_FAST_LOOP();
    float GetValveControl();
    void SetPressure(t_pressure_selector ps, float pressure);
    void SetTargetPressure(float pressure);
    void ConfigurePidSlow(float P, float I, float D, float pid_limiter);
    void ConfigurePidFast(float P, float I, float D);
    void GetPidSlow(float *P, float *I, float *D, float *pid_limiter);
    void GetPidFast(float *P, float *I, float *D);
    void SetPidFilter(float fast, float slow);
    void GetPidFilter(float *fast, float *slow);
    void GetPidMonitor(float* slow, float* fast);

private:
    float _PID_P2;
    float _PID_I2;
    float _PID_D2;

    float _PID_P;
    float _PID_I;
    float _PID_D;

    float _pid_limit;

    float _Pset;
    float _pressure_valve;
    float _pressure_patient;

    int32_t _LoopRatio;
    
    int32_t LoopCounter;

    float fast_pid_set = 0;

    float _filter_fast;
    float _filter_slow;

    float _ValvePWM;

    float _fast_ms;
    
    HW* hwi;
    DebugIfaceClass* dbg;

    uint64_t cycle_Loop_LT;

    uint64_t last_fast_ms;
    uint64_t last_slow_ms;
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
