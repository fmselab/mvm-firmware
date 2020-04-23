// driver_OxygenSensor.h

#ifndef _DRIVER_OXYGENSENSOR_h
#define _DRIVER_OXYGENSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "DriverContext.h"

typedef enum { OxygenSensorA } t_OxygenSensorModel;

class OxygenSensor
{
public:
	void Init(t_OxygenSensorModel model, void* handle);
	float GetConcentration();
	bool setData(float adc_oxygen, float temperature);
	void CalibrateAir();
	void CalibratePureOxygen();
	bool CheckNeedRecalibrate();
	void Tick();

private:

	HW* hwi;
	DebugIfaceClass* dbg;
	float calib_q;
	float calib_m;
	float _adc_oxygen;
	float _temperature;
	uint64_t calib_interval_seconds;
	uint64_t calib_second_counter;
	uint64_t second_counter;
	t_OxygenSensorModel _oxmodel;
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
