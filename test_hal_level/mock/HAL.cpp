/*
 * HAL.cpp
 *
 *  Created on: 26 apr 2020
 *      Author: AngeloGargantini
 */

#include "HAL.h"
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono;
/*HAL::HAL() {
 // TODO Auto-generated constructor stub

 }

 HAL::~HAL() {
 // TODO Auto-generated destructor stub
 }
 */
class HW_mock: public HW {
public:
	void PrintDebugConsole(String s) {
		std::cout << s << std::endl;
	}
	virtual bool Init() {
	}
	virtual bool I2CWrite(t_i2cdevices device, uint8_t *wbuffer, int wlength,
			bool stop) {
	}
	virtual bool I2CRead(t_i2cdevices device, uint8_t *wbuffer, int wlength,
			uint8_t *rbuffer, int rlength, bool stop) {
	}
	virtual bool I2CRead(t_i2cdevices device, uint8_t *rbuffer, int rlength,
			bool stop) {
	}
	virtual bool PWMSet(hw_pwm id, float value) {
	}
	virtual bool IOSet(hw_gpio id, bool value) {
	}
	virtual bool IOGet(hw_gpio id, bool *value) {
	}
	virtual void __delay_blocking_ms(uint32_t ms) {
	}
	virtual void PrintLineDebugConsole(String s) {
	}
	virtual void Tick() {
	}
	virtual uint64_t GetMillis() {
	}
	virtual int64_t Get_dT_millis(uint64_t ms) {
	}
	virtual bool DataAvailableOnUART0() {
	}
	virtual String ReadUART0UntilEOL() {
	}
	virtual bool WriteUART0(String s) {
	}
	virtual void GetPowerStatus(bool *batteryPowered, float *charge) {
	}
	virtual float GetPIN() {
	}
	virtual float GetBoardTemperature() {
	}
	virtual uint16_t GetSupervisorAlarms() {
	}
};

static HW_mock myHW;

void HAL::Init() {
	std::cout << "init hal" << std::endl;
	dbg.Init(DBG_ALL, &myHW);
	dbg.DbgPrint(DBG_CODE, DBG_INFO,
			"Calibrating pressure sensors. Idraulic circuit must be opened");
}

void HAL::Tick() {
	std::runtime_error("ERROR IN NAVIGATION");
}

float HAL::GetPressureValve(int32_t Delay) {
	// What it is? see limits
	return 10;
}
float HAL::GetPressurePatient(int32_t Delay) {
	// What it is? see limits
	return 10;
}
float HAL::GetFlowInspire(int32_t Delay) {
	return 10;
}
float HAL::GetFlowVenturi(int32_t Delay) {
	return 10;
}
float HAL::GetPVenturi(int32_t Delay) {
	throw std::runtime_error("3not implemented!");
}
void HAL::SetInputValve(float value) {
	this->InputValve = value;
	if (value > 0 && OutputValve)
		std::runtime_error("valve both opens");
}
float HAL::GetInputValve() {
	return this->InputValve;
}
void HAL::SetOutputValve(bool value) {
	this->OutputValve = value;
}
float HAL::GetOutputValve() {
	return this->OutputValve;
}
void HAL::SetBuzzer(bool value) {
	buzzer = value;
}
void HAL::SetAlarmLed(bool value) {
	alarmedled = value;
}
void HAL::SetAlarmRele(bool value) {
	alarmedrel = value;
}

float HAL::GetVolumeVenturi() {
	throw std::runtime_error("11 not implemented!");
}
float HAL::GetVolumeInput() {
	throw std::runtime_error("12 not implemented!");
}
void HAL::ResetVolumeVenturi() {
	throw std::runtime_error("13 not implemented!");
}
void HAL::ResetVolumeInput() {
	throw std::runtime_error("14 not implemented!");
}

bool HAL::DataAvailableOnUART0() {
	throw std::runtime_error("15 not implemented!");
}

String HAL::ReadUART0UntilEOL() {
	throw std::runtime_error("16 not implemented!");
}

bool HAL::WriteUART0(String s) {
	throw std::runtime_error("17 not implemented!");
}

void HAL::GetInputValvePID(float *pid_slow, float *pid_fast) {
	//throw std::runtime_error("18 not implemented!");
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
	return (int64_t) (GetMillis() - ms);
}

float HAL::ZeroPressureSensor(t_pressure_sensor ps) {
	return 0;
}

void HAL::SetZeroPressureSensor(t_pressure_sensor ps, float value) {
	throw std::runtime_error("21 not implemented!");
}

void HAL::CorrectZeroPressureSensor(t_pressure_sensor ps, float value) {
	throw std::runtime_error("22 not implemented!");
}
void HAL::ConfigureInputValvePID(float P, float I, float D, float P2, float I2,
		float D2, float pid_limit) {
	throw std::runtime_error("23 not implemented!");
}

void HAL::delay_ms(float ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds((int) ms));
}

float HAL::GetOxygen() {
	return 10;
	//throw std::runtime_error("25 not implemented!");
}
void HAL::CalibrateOxygenSensorInAir() {
	throw std::runtime_error("26 not implemented!");
}

void HAL::CalibrateOxygenSensorInPureOxygen() {
	throw std::runtime_error("26 not implemented!");
}

void HAL::TriggerAlarm(t_ALARM alarm_code) {
	throw std::runtime_error("28 not implemented!");
}

float HAL::GetGasTemperature() {
	throw std::runtime_error("29 not implemented!");
}

void HAL::GetPowerStatus(bool *batteryPowered, float *charge) {
	// for now use corrent
	*batteryPowered = false;
}

void HAL::FlushPipes(bool run, float valve) {
	throw std::runtime_error("31 not implemented!");
}

void HAL::DOVenturiMeterScan() {
	throw std::runtime_error("32 not implemented!");
}

