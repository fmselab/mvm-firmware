// HAL.h

#ifndef _HAL_h
#define _HAL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "generic_definitions.h"

#ifdef HARDWARE_TARGET_PLATFORM_V3
#include "fw_board_razzeto_v3.h"
#endif
#ifdef  HARDWARE_TARGET_PLATFORM_V4
#include "fw_board_ni_v4.h"
#endif



#include "DebugIface.h"
#include "driver_5525DSO.h"
#include "driver_SFM3019.h"
#include "driver_ADS1115.h"
#include "driver_VenturiFlowMeter.h"
#include "driver_OxygenSensor.h"
#include "PressureLoop.h"
#include "CircularBuffer.h"


class HAL
{
	private:
#ifdef HARDWARE_TARGET_PLATFORM_V3
		HW_V3 hwi;
#endif
#ifdef  HARDWARE_TARGET_PLATFORM_V4
		HW_V4 hwi;
#endif

		Sensor5525DSO drv_PLoop;
		Sensor5525DSO drv_PPatient;
		Sensor5525DSO drv_PVenturi;
		SensorSFM3019 drv_FlowIn;
		ADC_ADS1115 drv_ADC0;
		VenturiFlowMeter drv_FlowVenturi;
		OxygenSensor drv_OxygenSensor;
		PressureLoopClass PressureLoop;
		DriverContext _dc;
		CircularBuffer *MEM_PLoop;
		CircularBuffer *MEM_PPatient;
		CircularBuffer *MEM_FlowIn;
		CircularBuffer *MEM_FlowVenturi;
		CircularBuffer* MEM_PVenturi;


		uint64_t cycle_PLoop_LT;
		uint64_t cycle_PPatient_LT;
		uint64_t cycle_PVenturi_LT;
		uint64_t cycle_FlowIn_LT;
		uint64_t cycle_ADC_LT;
		uint64_t cycle_Supervisor_LT;
		uint64_t cycle_LT;
		float Tloop, Ploop;
		float Tpatient, Ppatient;
		float FlowIn, TFlowIn;
		float Tventuri, Pventuri;
		float FlowVenturi;
		float Oxygen;
		float VoltageReference;
		float VoltageProbe12V;
		float VoltageProbe5V;
		float _InputValveValue;
		float _OutputValveValue;
		float GasTemperature;
		
		float Pin;
		float BoardTemperature;
		uint16_t SupervisorAlarms;

		bool flush_pipe_mode = false;
		float flush_pipe_open = 0;

	public:
		DebugIfaceClass dbg;
		void Init();
		float GetPressureValve(int32_t Delay);
		float GetPressurePatient(int32_t Delay);
		float GetFlowInspire(int32_t Delay);
		float GetFlowVenturi(int32_t Delay);
		float GetPVenturi(int32_t Delay);
		void SetInputValve(float value);
		float GetInputValve();
		void SetOutputValve(bool value);
		float GetOutputValve();
		void GetInputValvePID(float *pid_slow, float *pid_fast);
		void ConfigureInputValvePID(float P, float I, float D, float P2, float I2, float D2, float pid_limit);
		void SetBuzzer(bool value);
		void SetAlarmLed(bool value);
		void SetAlarmRele(bool value);
		float GetVolumeVenturi();
		float GetVolumeInput();
		void ResetVolumeVenturi();
		void ResetVolumeInput();
		bool DataAvailableOnUART0();
		String ReadUART0UntilEOL();
		bool WriteUART0(String s);
		uint64_t GetMillis();
		int64_t Get_dT_millis(uint64_t ms);
		float ZeroPressureSensor(t_pressure_sensor ps);
		void SetZeroPressureSensor(t_pressure_sensor ps, float value);
		void CorrectZeroPressureSensor(t_pressure_sensor ps, float value);
		void delay_ms(float ms);
		float GetOxygen();
		void CalibrateOxygenSensorInAir();
		void CalibrateOxygenSensorInPureOxygen();
		void TriggerAlarm(t_ALARM alarm_code);
		float GetGasTemperature();
		void GetPowerStatus(bool* batteryPowered, float* charge);
		void FlushPipes(bool run, float valve);
		void DOVenturiMeterScan();
		uint8_t i2c_scheduler;
		uint8_t _adc_channel;
		float ADC_Results[4];

		std::function<void()> callback_ploop = NULL;
		std::function<void()> callback_ppatient = NULL;
		std::function<void()> callback_flowsens = NULL;
		std::function<void()> callback_venturi = NULL;
		
		std::function<void(t_ALARM alarm_code)> callback_alarm = NULL;

		void addHandler_PLoop(std::function<void()> callback)
		{
			callback_ploop = callback;
		}

		void addHandler_PPatient(std::function<void()> callback)
		{
			callback_ppatient = callback;
		}

		void addHandler_FlowSens(std::function<void()> callback)
		{
			callback_flowsens = callback;
		}

		void addHandler_FlowVenturi(std::function<void()> callback)
		{
			callback_venturi = callback;
		}

		void addHandler_HardwareAlarm(std::function<void(t_ALARM alarm_code)> callback)
		{
			callback_alarm = callback;
		}


		void Tick();
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
