// 
// 
// 

#include "driver_VenturiFlowMeter.h"

void VenturiFlowMeter::Init(t_VenturiSensorModel model)
{
	_LowPass = 0.8;
	_model = model;
	VenturiFlux = 0;
	Integral = 0;

}

float VenturiFlowMeter::GetFlow(float pressure, float temperature)
{
	if (_model == SpiroquantH_R122P04)
	{
		float vf = SpiroquantH_R122P04_Convert(pressure);
		VenturiFlux = (_LowPass * VenturiFlux) + ((1 - _LowPass) * vf);
		return VenturiFlux;
	}

	if (_model == ALPE_1551)
	{
		float vf = ALPE_1551_Convert(pressure);
		VenturiFlux = (_LowPass * VenturiFlux) + ((1 - _LowPass) * vf);
		return VenturiFlux;
	}
}
float VenturiFlowMeter::SpiroquantH_R122P04_Convert(float pressure)
{
	float dp = pressure;
	float sign = 1;
	if (dp < 0) {
		sign = -1;
		dp = dp * -1;
	};
	float vf= 0.1513 * (dp * dp * dp) - 3.3424 * (dp * dp) + 41.657 * dp;
	vf = vf * sign;
	Integral += vf;
	return vf;
}
float VenturiFlowMeter::ALPE_1551_Convert(float pressure)
{
	float dp = pressure;
	float sign = 1;
	if (dp < 0) {
		sign = -1;
		dp = dp * -1;
	};
	float vf = -0.0647130 * (dp * dp * dp * dp) + 1.4481369 * (dp * dp * dp)
		- 11.2548243 * (dp * dp) + 43.6025093 * dp;
	vf = vf * sign;


	Integral += vf;
	return vf;
}

bool VenturiFlowMeter::setLowpass(float lowpass)
{
	if ((lowpass >= 0) && (lowpass <= 1))
	{
		_LowPass = lowpass;
		return true;
	}
	else
		return false;
}

float VenturiFlowMeter::GetIntegral()
{
	return Integral;
}
void VenturiFlowMeter::ResetIntegral()
{
	Integral = 0;
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
