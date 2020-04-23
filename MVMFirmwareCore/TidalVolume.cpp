// 
// 
// 

#include "TidalVolume.h"
#include <math.h>
#define VOL_COMP 0.4
void TidalVolumeClass::Init(HAL* hal)
{
	TotalVolume=0;
	InspVolumeSensirion=0;
	InspVolumeVenturi=0;
	ExpVolumeVenturi=0;
	TidalCorrection = 1;
	FLUX=0;
	Status = 0;
	_HAL = hal;
}
void TidalVolumeClass::PushDataSens(float flux_sens)
{
	
	float dT;
	dT = ((float)_HAL->Get_dT_millis(last_meas_t))/1000.0;
	last_meas_t = _HAL->GetMillis();

	if (Status==1)
	{ 
		TotalVolume += flux_sens* dT / 60.0;
		InspVolumeSensirion += flux_sens* dT/60.0;
		FLUX = flux_sens;
		FluxMax = flux_sens > FluxMax ? flux_sens : FluxMax;
		liveFlux = FLUX;
		liveVolume = TotalVolume;
	}
	else
	{
		if (Status == 2)
		{

		}
		else
		{
			liveFlux = flux_sens;
		}
	}
}
void TidalVolumeClass::PushDataVenturi(float flux_venturi)
{
	float dT;
	dT = ((float)_HAL->Get_dT_millis(last_meas_t_b)) / 1000.0;
	last_meas_t_b = _HAL->GetMillis();

	if (Status == 1)
	{
		InspVolumeVenturi += flux_venturi * dT / 60.0;
	}
	else
	{
		if (Status == 2)
		{
			ExpVolumeVenturi += flux_venturi * dT / 60.0;
			if (TidalCorrection > 0) {
				float vf_clamp=0;
				vf_clamp = fabs(flux_venturi) > 0.5 ? flux_venturi : 0;
				TotalVolume += (vf_clamp *  TidalCorrection) * dT / 60.0;
				FLUX = flux_venturi  * TidalCorrection;
				liveFlux = FLUX;
				liveVolume = TotalVolume;
			}
		}
		else
		{
			
			//do nothing
		}
	}
}
void TidalVolumeClass::DoNewCycle()
{
	TotalVolume = 0;
	InspVolumeSensirion = 0;
	InspVolumeVenturi = 0;
	ExpVolumeVenturi = 0;
	TidalCorrection = 1;
	FLUX = 0;
	Status = 1;
	FluxMax = 0;
	last_meas_t = _HAL->GetMillis();
	last_meas_t_b = _HAL->GetMillis();
}
void TidalVolumeClass::DoExhale()
{
	currentTvIsnp = InspVolumeSensirion;
	currentFluxPeak = FluxMax;
	
	if (InspVolumeSensirion > 0)
		TidalCorrection = InspVolumeVenturi / InspVolumeSensirion;
	else
		TidalCorrection = 1;
	//Serial.println("SENS: " + String(InspVolumeVenturi) + " VENT: " + String(InspVolumeSensirion) + " COR: " + String(TidalCorrection));
	Status = 2;
}
void TidalVolumeClass::DoEndCycle()
{
	if (TidalCorrection > 0) {
		currentTvEsp = -1.0 * ExpVolumeVenturi * TidalCorrection;
	}
	Status = 0;
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
