// 
// 
// 

#include "driver_OxygenSensor.h"


void OxygenSensor::Init(t_OxygenSensorModel model, void* handle)
{
	DriverContext* dc;
	dc = (DriverContext*)handle;
	hwi = (HW*)dc->hwi;
	dbg = (DebugIfaceClass*)dc->dbg;

	_oxmodel = model;
	if (_oxmodel == OxygenSensorA)
	{
		calib_q = 0.00452689;
		calib_m = -1.63;
		calib_interval_seconds = 3600 * 24 * 7;		//7 days
	}
	
	
	_adc_oxygen=0;
	_temperature=0;
	calib_second_counter=0;
	second_counter=0;

	second_counter = hwi->GetMillis();
}
float OxygenSensor::GetConcentration()
{
	float o2;
	o2 = _adc_oxygen * calib_q + calib_m;
	o2 = o2 > 100 ? 100 : o2;
	return o2;

}
bool OxygenSensor::setData(float adc_oxygen, float temprature)
{
	_adc_oxygen = adc_oxygen;
	_temperature = temprature;
}
void OxygenSensor::CalibrateAir()
{
	calib_m = -(_adc_oxygen - 21);
	calib_second_counter = 0;
}
void OxygenSensor::CalibratePureOxygen()
{
	calib_m = -(_adc_oxygen - 100);
	calib_second_counter = 0;
}

bool OxygenSensor::CheckNeedRecalibrate()
{
	if (calib_second_counter > calib_interval_seconds)
		return true;
	else
		return false;
}
void OxygenSensor::Tick()
{
	if (hwi->Get_dT_millis(second_counter) > 1000)
	{
		calib_second_counter++;
	}
}


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
