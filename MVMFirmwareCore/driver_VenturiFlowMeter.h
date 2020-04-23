// driver_VenturiSpiroquantH .h

#ifndef _DRIVER_VENTURISPIROQUANTH _h
#define _DRIVER_VENTURISPIROQUANTH _h
typedef enum { SpiroquantH_R122P04, ALPE_1551} t_VenturiSensorModel;

class VenturiFlowMeter
{
public:
	void Init(t_VenturiSensorModel model);
	float GetFlow(float pressure, float temperature);
	bool setLowpass(float lowpass);
	float GetIntegral();
	void ResetIntegral();


private:
	float SpiroquantH_R122P04_Convert(float pressure);
	float ALPE_1551_Convert(float pressure);
	float VenturiFlux=0;
	float _LowPass;
	float Integral=0;
	t_VenturiSensorModel _model;
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
