/*
 * HAL.cpp
 *
 *  Created on: 26 apr 2020
 *      Author: AngeloGargantini
 */

#include "HAL.h"
#include <chrono>

using namespace std::chrono;
/*HAL::HAL() {
 // TODO Auto-generated constructor stub

 }

 HAL::~HAL() {
 // TODO Auto-generated destructor stub
 }
 */

void HAL::Init() {
}

void HAL::Tick() {
}

float HAL::GetPressureValve(int32_t Delay) {
}
float HAL::GetPressurePatient(int32_t Delay) {
}
float HAL::GetFlowInspire(int32_t Delay) {
}
float HAL::GetFlowVenturi(int32_t Delay) {
}
float HAL::GetPVenturi(int32_t Delay) {
}
void HAL::SetInputValve(float value) {
}
float HAL::GetInputValve() {
}
void HAL::SetOutputValve(bool value) {
}
float HAL::GetOutputValve() {
}
void HAL::SetBuzzer(bool value) {
}
void HAL::SetAlarmLed(bool value) {
}
void HAL::SetAlarmRele(bool value) {
}

float HAL::GetVolumeVenturi() {
}
float HAL::GetVolumeInput() {
}
void HAL::ResetVolumeVenturi() {
}
void HAL::ResetVolumeInput() {
}

bool HAL::DataAvailableOnUART0() {
}

String HAL::ReadUART0UntilEOL() {
}

bool HAL::WriteUART0(String s) {
}

void HAL::GetInputValvePID(float *pid_slow, float *pid_fast) {
}

uint64_t HAL::GetMillis() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
// used in Alarm
unsigned long millis() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int64_t HAL::Get_dT_millis(uint64_t ms) {
}

float HAL::ZeroPressureSensor(t_pressure_sensor ps) {
}

void HAL::SetZeroPressureSensor(t_pressure_sensor ps, float value) {
}

void HAL::CorrectZeroPressureSensor(t_pressure_sensor ps, float value) {
}
void HAL::ConfigureInputValvePID(float P, float I, float D, float P2, float I2,
		float D2, float pid_limit) {
}

void HAL::delay_ms(float ms) {
}

float HAL::GetOxygen() {
}
void HAL::CalibrateOxygenSensorInAir() {
}

void HAL::CalibrateOxygenSensorInPureOxygen() {
}

void HAL::TriggerAlarm(t_ALARM alarm_code) {
}

float HAL::GetGasTemperature() {
}

void HAL::GetPowerStatus(bool *batteryPowered, float *charge) {
}

void HAL::FlushPipes(bool run, float valve) {
}

void HAL::DOVenturiMeterScan() {
}

