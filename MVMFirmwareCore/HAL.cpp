// 
// 
// 


#define SCHEDULER_TIMER_PLOOP			5
#define SCHEDULER_TIMER_PPATIENT		20
#define SCHEDULER_TIMER_FLOWIN			20
#define SCHEDULER_TIMER_PVENTURI		20
#define SCHEDULER_TIMER_ADC				1000
#define SCHEDULER_TIMER_SUPERVISOR		1000



#include "HAL.h"
#include <functional>

void HAL::Init()
{

	hwi.Init();
	dbg.Init(DBG_WARNING, &hwi);
	_dc.hwi = &hwi;
	_dc.dbg = &dbg;
	
	drv_FlowIn.Init(IIC_FLOW1, &_dc);

	drv_PLoop.Init(IIC_PS_0, PLOOP_MODEL, OVS_1024, &_dc);
	drv_PPatient.Init(IIC_PS_1, PPATIENT_MODEL, OVS_1024, &_dc);
	drv_PVenturi.Init(IIC_PS_2, PVENTURI, OVS_1024, &_dc);
	drv_FlowVenturi.Init(ALPE_1551);

	PressureLoop.Init(5, 2, &_dc);
	drv_ADC0.Init(IIC_ADC_0, &_dc);
	drv_OxygenSensor.Init(OxygenSensorA, &_dc);
	drv_ADC0.setGain(GAIN_ONE);

	MEM_PLoop = new CircularBuffer(4);
	MEM_PPatient = new CircularBuffer(16);
	MEM_FlowIn = new CircularBuffer(4);
	MEM_FlowVenturi = new CircularBuffer(4);
	MEM_PVenturi = new CircularBuffer(8);

	cycle_PLoop_LT = hwi.GetMillis();
	cycle_PPatient_LT = hwi.GetMillis();
	cycle_FlowIn_LT = hwi.GetMillis();
	cycle_PVenturi_LT = hwi.GetMillis();
	cycle_ADC_LT = hwi.GetMillis();
	cycle_Supervisor_LT = hwi.GetMillis();
	cycle_LT = hwi.GetMillis();

	_adc_channel = 0;

	Tloop = 0;
	Ploop = 0;
	Tpatient = 0; 
	Ppatient = 0;
	FlowIn = 0;
	TFlowIn = 0;
	Tventuri = 0;
	Pventuri = 0;
	FlowVenturi = 0;
	Oxygen = 0;
	VoltageReference = 2.5;
	VoltageProbe12V = 12;
	VoltageProbe5V = 5;
	_InputValveValue = 0;
	_OutputValveValue = 0;
	GasTemperature = 0;

	Pin = 0;
	BoardTemperature = 0;
	SupervisorAlarms=0;
	i2c_scheduler = 0;
	flush_pipe_mode = false;

	//hwi->addHandler(std::bind(&HAL::Callback, this, 1));
}
/*
void HAL::Callback(int x)
{
	dbg->DbgPrint(DBG_CODE, DBG_CRITICAL, String((int32_t)hwi->GetMillis()) + " - CALLBACK CALLED " + String(x));
}*/

void HAL::Tick()
{
	uint32_t ADC_LastResult;

	if (flush_pipe_mode == false)
	{
		PressureLoop.Tick();
		hwi.PWMSet(PWM_PV1, PressureLoop.GetValveControl());
	}
	else
	{
		hwi.PWMSet(PWM_PV1, flush_pipe_open);
		SetOutputValve(true);
	}

	
	
	if (hwi.Get_dT_millis(cycle_LT) >= 3)
	{
		

		if (drv_PLoop.asyncGetResult(&Ploop, &Tloop))
		{
			if (fabs(Ploop) < 150)
			{
				MEM_PLoop->PushData(Ploop);
				PressureLoop.SetPressure(PRESSURE_VALVE, Ploop);
				dbg.DbgPrint(DBG_CODE, DBG_VALUE, String((int32_t)hwi.GetMillis()) + " - Ploop: " + String(Ploop));
				if (callback_ploop)
					callback_ploop();
			}
		}
		else if (drv_PPatient.asyncGetResult(&Ppatient, &Tpatient))
		{
			if (fabs(Ploop) < 150)
			{
				MEM_PPatient->PushData(Ppatient);
				PressureLoop.SetPressure(PRESSURE_PATIENT, Ppatient);
				dbg.DbgPrint(DBG_CODE, DBG_VALUE, String((int32_t)hwi.GetMillis()) + " - PPatient: " + String(Ppatient));
				if (callback_ppatient)
					callback_ppatient();
			}
		}
		else if (drv_PVenturi.asyncGetResult(&Pventuri, &Tventuri))
		{
			if (fabs(Pventuri) < 10)
			{
				FlowVenturi = drv_FlowVenturi.GetFlow(Pventuri, Tventuri);
				MEM_FlowVenturi->PushData(FlowVenturi);
				MEM_PVenturi->PushData(Pventuri);
				dbg.DbgPrint(DBG_CODE, DBG_VALUE, String((int32_t)hwi.GetMillis()) + " - PVenturi: " + String(Pventuri) + " - FlowVenturi: " + String(FlowVenturi));
				if (callback_venturi)
					callback_venturi();
			}
		}
		else if (drv_ADC0.asyncGetResult(&ADC_LastResult))
		{
			ADC_Results[_adc_channel] = ADC_LastResult;
#ifdef HARDWARE_TARGET_PLATFORM_V4
			if (_adc_channel == 0)
			{
				drv_OxygenSensor.setData(ADC_Results[_adc_channel], Tloop);
				Oxygen = drv_OxygenSensor.GetConcentration();
			}

			if (_adc_channel == 1)
			{
				VoltageReference = ADC_Results[_adc_channel];

				//TODO: Check value for alarm
			}

			if (_adc_channel == 2)
			{
				VoltageProbe12V = ((float)ADC_Results[_adc_channel]) / VoltageReference * 2.5 * 5;
				if ((VoltageProbe12V < 10) || (VoltageProbe12V > 15))
					TriggerAlarm(ALARM_OVER_UNDER_VOLTAGE);
			}

			if (_adc_channel == 3)
			{
				VoltageProbe5V = ((float)ADC_Results[_adc_channel]) / VoltageReference * 2.5 * 2;
				if ((VoltageProbe5V < 4.7) || (VoltageProbe5V > 5.3))
					TriggerAlarm(ALARM_OVER_UNDER_VOLTAGE);
			}

			_adc_channel++;
			if (_adc_channel > 3) _adc_channel = 0;
#endif
		}
		else
		{
			switch (i2c_scheduler)
			{
			case 0:
				if (hwi.Get_dT_millis(cycle_PLoop_LT) > SCHEDULER_TIMER_PLOOP)
				{
					cycle_PLoop_LT = hwi.GetMillis();
					if (!drv_PLoop.asyncMeasure())
						TriggerAlarm(UNABLE_TO_READ_SENSOR_PRESSURE);
				}
				i2c_scheduler = 1;
				break;

			case 1:
				if (hwi.Get_dT_millis(cycle_PPatient_LT) > SCHEDULER_TIMER_PPATIENT)
				{
					cycle_PPatient_LT = hwi.GetMillis();
					if (!drv_PPatient.asyncMeasure())
						TriggerAlarm(UNABLE_TO_READ_SENSOR_PRESSURE);
				}
				i2c_scheduler = 2;
				break;

			case 2:
				if (hwi.Get_dT_millis(cycle_PVenturi_LT) > SCHEDULER_TIMER_PVENTURI)
				{

					cycle_PVenturi_LT = hwi.GetMillis();
					if (!drv_PVenturi.asyncMeasure())
						TriggerAlarm(UNABLE_TO_READ_SENSOR_VENTURI);
				}
				i2c_scheduler = 3;
				break;

			case 3:
				if (hwi.Get_dT_millis(cycle_ADC_LT) > SCHEDULER_TIMER_ADC)
				{
					cycle_ADC_LT = hwi.GetMillis();
					drv_ADC0.asyncMeasure(_adc_channel);
				}
				i2c_scheduler = 4;
				break;
			case 4:
				if (hwi.Get_dT_millis(cycle_Supervisor_LT) > SCHEDULER_TIMER_SUPERVISOR)
				{
					cycle_Supervisor_LT = hwi.GetMillis();

					Pin = hwi.GetPIN();
					BoardTemperature = hwi.GetBoardTemperature();
					SupervisorAlarms = hwi.GetSupervisorAlarms();


					//Check supervisors alarms
					if (SupervisorAlarms != 0)
					{
						TriggerAlarm(ALARM_SUPERVISOR);
					}

					//Pressure Alarms
					if (Pin > MAX_PIN)
					{
						TriggerAlarm(ALARM_PRESSURE_INPUT_TOO_HIGH);
					}

					if ((_InputValveValue > 0) && (Pin < MIN_PIN))
					{
						TriggerAlarm(ALARM_PRESSURE_INPUT_TOO_LOW);
					}

					//Check board temperature
					if (BoardTemperature > 75)
					{
						TriggerAlarm(ALARM_OVERTEMPERATURE);
					}

				}
				i2c_scheduler = 5;
				break;

			case 5:
				if (hwi.Get_dT_millis(cycle_FlowIn_LT) > SCHEDULER_TIMER_FLOWIN)
				{
					cycle_FlowIn_LT = hwi.GetMillis();
					if (!drv_FlowIn.doMeasure(&FlowIn, &TFlowIn))
						TriggerAlarm(UNABLE_TO_READ_SENSOR_FLUX);
					MEM_FlowIn->PushData(FlowIn);
					GasTemperature = TFlowIn;
					dbg.DbgPrint(DBG_CODE, DBG_VALUE, String((int32_t)hwi.GetMillis()) + " - Flow: " + String(FlowIn));
					if (callback_flowsens)
						callback_flowsens();
				}
				i2c_scheduler = 6;
				break;
			case 6:
				hwi.Tick();
				i2c_scheduler = 0;
				break;
			}
		}

		cycle_LT = hwi.GetMillis();
		

		
		

		
		
	}
}


float HAL::GetPressureValve(int32_t Delay)
{
	return MEM_PLoop->GetData(Delay);
}
float HAL::GetPressurePatient(int32_t Delay)
{
	return MEM_PPatient->GetData(Delay);
}
float HAL::GetFlowInspire(int32_t Delay)
{
	return MEM_FlowIn->GetData(Delay);
}
float HAL::GetFlowVenturi(int32_t Delay)
{
	return MEM_FlowVenturi->GetData(Delay);
}
float HAL::GetPVenturi(int32_t Delay)
{
	return MEM_PVenturi->GetData(Delay);
}
void HAL::SetInputValve(float value)
{
	_InputValveValue = value;
	PressureLoop.SetTargetPressure(value);
}
float HAL::GetInputValve()
{
	return 	_InputValveValue;
}
void HAL::SetOutputValve(bool value)
{
	_OutputValveValue = value ? 1:0;
	hwi.IOSet(GPIO_PV2, !value);
}
float HAL::GetOutputValve()
{
	return _OutputValveValue;
}
void HAL::SetBuzzer(bool value)
{
	hwi.IOSet(GPIO_BUZZER, value);
}
void HAL::SetAlarmLed(bool value)
{
	hwi.IOSet(GPIO_LED, value);
}
void HAL::SetAlarmRele(bool value)
{
	hwi.IOSet(GPIO_RELEALLARM, value);
}

float HAL::GetVolumeVenturi()
{
	return drv_FlowVenturi.GetIntegral();
}
float HAL::GetVolumeInput()
{
	return drv_FlowIn.GetIntegral();
}
void HAL::ResetVolumeVenturi()
{
	drv_FlowVenturi.ResetIntegral();
}
void HAL::ResetVolumeInput()
{
	drv_FlowIn.ResetIntegral();
}

bool HAL::DataAvailableOnUART0()
{
	return hwi.DataAvailableOnUART0();
}

String HAL::ReadUART0UntilEOL()
{
	return hwi.ReadUART0UntilEOL();
}

bool HAL::WriteUART0(String s)
{
	return hwi.WriteUART0(s);
}

void HAL::GetInputValvePID(float* pid_slow, float* pid_fast)
{
	PressureLoop.GetPidMonitor(pid_slow, pid_fast);
}

uint64_t HAL::GetMillis()
{
	return hwi.GetMillis();
}

int64_t HAL::Get_dT_millis(uint64_t ms)
{
	return hwi.Get_dT_millis(ms);
}

float HAL::ZeroPressureSensor(t_pressure_sensor ps)
{
	
	switch (ps)
	{
	case PS_LOOP:
		return drv_PLoop.doZero();
		break;
	case PS_PATIENT:
		return drv_PPatient.doZero();
		break;
	case PS_VENTURI:
		return drv_PVenturi.doZero();
		break;
	default:
		return 0;
		break;
	}
}

void HAL::SetZeroPressureSensor(t_pressure_sensor ps, float value)
{
	switch (ps)
	{
	case PS_LOOP:
		drv_PLoop.setZero(value);
		break;
	case PS_PATIENT:
		drv_PPatient.setZero(value);
		break;
	case PS_VENTURI:
		drv_PVenturi.setZero(value);
		break;
	default:
		break;
	}
}


void HAL::CorrectZeroPressureSensor(t_pressure_sensor ps, float value)
{
	switch (ps)
	{
	case PS_LOOP:
		drv_PLoop.correctZero(value);
		break;
	case PS_PATIENT:
		drv_PPatient.correctZero(value);
		break;
	case PS_VENTURI:
		drv_PVenturi.correctZero(value);
		break;
	default:
		break;
	}
}
void HAL::ConfigureInputValvePID(float P, float I, float D, float P2, float I2, float D2, float pid_limit)
{
	PressureLoop.ConfigurePidSlow(P2, I2, D2, pid_limit);
	PressureLoop.ConfigurePidFast(P, I, D);
}

void HAL::delay_ms(float ms)
{
	hwi.__delay_blocking_ms((uint32_t) ms);
}

float HAL::GetOxygen()
{
	return Oxygen;
}
void HAL::CalibrateOxygenSensorInAir()
{
	drv_OxygenSensor.CalibrateAir();
}

void HAL::CalibrateOxygenSensorInPureOxygen()
{
	drv_OxygenSensor.CalibratePureOxygen();
}

void HAL::TriggerAlarm(t_ALARM alarm_code)
{
	if (callback_alarm)
		callback_alarm(alarm_code);
}

float HAL::GetGasTemperature()
{
	return GasTemperature;
}

void HAL::GetPowerStatus(bool* batteryPowered, float* charge)
{
	hwi.GetPowerStatus(batteryPowered, charge);
}

void HAL::FlushPipes(bool run, float valve)
{
	flush_pipe_mode = run;
	flush_pipe_open = valve;
}


void HAL::DOVenturiMeterScan()
{
	float fref, tref;
	float pmeas, tmeas;
	float fref_m, pmeas_m, cnt;
	if (flush_pipe_mode)
	{
		SetOutputValve(true);
		for (int i = 30;i < 100;i++)
		{
			hwi.PWMSet(PWM_PV1, i);
			hwi.__delay_blocking_ms(500);
			fref_m = 0;
			pmeas_m = 0;
			for (int j = 0;j < 30;j++)
			{
				drv_FlowIn.doMeasure(&fref, &tref);
				drv_PVenturi.doMeasure(&pmeas, &tmeas);
				fref_m += fref;
				pmeas_m += pmeas;
			}
			
			fref_m = fref_m / 30;
			pmeas_m = pmeas_m / 30;

			hwi.WriteUART0(String(i) + "," + String(fref_m,5) + "," + String(pmeas_m,5));
		}
		
		
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
