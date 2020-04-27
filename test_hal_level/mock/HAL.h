/*
 * HAL.h
 *
 *  Created on: 26 apr 2020
 *      Author: AngeloGargantini
 */

#ifndef HAL_H_
#define HAL_H_

#include "DebugIface.h"

class HAL {

	private:
	float InputValve;
	bool OutputValve;
	bool alarmedled, alarmedrel;
	bool buzzer;

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
	void Tick();
};

#endif /* HAL_H_ */
