// driver_5525DSO.h

#ifndef _DRIVER_5525DSO_h
#define _DRIVER_5525DSO_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "DriverContext.h"

typedef struct
{
	int32_t C[6];
	int32_t Q[6];
	float ZERO;
} t_5525DSO_calibration_table;

//#define PBUFFER_SIZE 32
typedef enum { DS_01, GS_05 } t_ps_sensor;
typedef enum { OVS_256, OVS_512, OVS_1024, OVS_2048, OVS_4096 } t_ps_resolution;

	class Sensor5525DSO
	{
		public:

			
			bool Init(t_i2cdevices device, t_ps_sensor model, t_ps_resolution ps_resolution, void* hw_handle);
			bool doMeasure(float* P, float* T);

			bool asyncMeasure();
			bool asyncGetResult(float *P, float *T);

			void setZero(float value);
			float doZero();
			void correctZero(float value);
			float GetConversionDelay();

		private:



			t_5525DSO_calibration_table sensorCT;
			t_i2cdevices i2c_device;
			t_ps_sensor sensor_model;
			t_ps_resolution sensor_resolution;
			
			void CalibrateDate_5525DSO(int32_t raw_temp, int32_t raw_pressure, float* T, float* P);
			bool Reset_5525DSO();
			uint8_t GetResolutionByteCodeTemp();
			uint8_t GetResolutionByteCodePressure();
			uint32_t GetResolutionDelay();


			HW* hwi;
			DebugIfaceClass* dbg;

			uint32_t __chache_P;
			uint32_t __chache_T;
			int __TDiv;
			bool __last_is_T;
			bool __pending_meas;
			uint64_t __last_millis;
			//float PBuffer[PBUFFER_SIZE];
			int startup_counter;
			bool data_valid;

			bool _initialized = false;
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
