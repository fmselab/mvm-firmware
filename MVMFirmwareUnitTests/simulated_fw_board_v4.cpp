// 
// File: simulated_fw_board_v4.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 23-Apr-2020 Initial version.
//
// Description:
// Software simulation of a generic MVM firmware board.
//

#include <ctime>

#include "mvm_fw_unit_test_config.h"
test_hardware_t FW_TEST_hardware;

#include "simulated_fw_board_v4.h"

unsigned long millis()
{
  timespec res;
  ::clock_gettime(CLOCK_REALTIME, &res);
  unsigned long ret = (res.tv_sec * 1000) + (res.tv_nsec / 1000000);
  return ret;
}

void delay(uint32_t ms_wait)
{
  timespec res = { ms_wait/1000, (ms_wait % 1000) * 1000000 };
  ::nanosleep(&res, NULL);
}

bool
HW_V4::Init()

{
  sim_i2c_devaddr dadd;
  dadd.muxport = 0; dadd.address = 0x76;
  FW_TEST_hardware.insert(std::make_pair(dadd, TEST_TE_MS5525DSO));
  m_dev_addrs.insert(std::make_pair(IIC_PS_0, dadd));

  dadd.muxport = 0; dadd.address = 0x77;
  FW_TEST_hardware.insert(std::make_pair(dadd, TEST_TE_MS5525DSO));
  m_dev_addrs.insert(std::make_pair(IIC_PS_1, dadd));

  dadd.muxport = 1; dadd.address = 0x76;
  FW_TEST_hardware.insert(std::make_pair(dadd, TEST_TE_MS5525DSO));
  m_dev_addrs.insert(std::make_pair(IIC_PS_2, dadd));

  dadd.muxport = 1; dadd.address = 0x2e;
  FW_TEST_hardware.insert(std::make_pair(dadd, TEST_SENSIRION_SFM3019));
  m_dev_addrs.insert(std::make_pair(IIC_FLOW1, dadd));

  dadd.muxport = 4; dadd.address = 0x48;
  FW_TEST_hardware.insert(std::make_pair(dadd, TEST_TI_ADS1115));
  m_dev_addrs.insert(std::make_pair(IIC_ADC_0, dadd));

  dadd.muxport = 3; dadd.address = 0x22;
  FW_TEST_hardware.insert(std::make_pair(dadd, TEST_XXX_SUPERVISOR));
  m_dev_addrs.insert(std::make_pair(IIC_SUPERVISOR, dadd));

  dadd.muxport = -1; dadd.address = 0x70;
  FW_TEST_hardware.insert(std::make_pair(dadd, TEST_TCA_I2C_MULTIPLEXER));
  m_dev_addrs.insert(std::make_pair(IIC_MUX, dadd));

  /* Broadcast address */
  dadd.muxport = 1; dadd.address = 0x00;
  m_dev_addrs.insert(std::make_pair(IIC_GENERAL_CALL_SENSIRION, dadd));

  for (int i = 0; i < 8; i++)
   {
    i2c_MuxSelect(i);
    Serial.println("SCAN I2C BUS: " + String(i));
    __service_i2c_detect();
   }

  batteryStatus_reading_LT = GetMillis();

  currentBatteryCharge = 100;
  pWall=true;
  pIN=3;
  BoardTemperature=25;
  //init supervisor watchdog
  WriteSupervisor(0x00, 0);  //REMOVE COMMENT BEFORE RELEASE

  return true;
}

bool
HW_V4::I2CWrite(t_i2cdevices device, uint8_t* wbuffer, int wlength, bool stop)
{
  sim_i2c_devaddr dad=m_dev_addrs[device];
  int ret;
  if ((ret = m_sim_devs.exchange_message(dad, wbuffer, wlength,
                                         NULL, 0, stop)) < 0)
   {
    return false;
   }

  return true;
}

bool
HW_V4::I2CRead(t_i2cdevices device, uint8_t* wbuffer, int wlength, uint8_t* rbuffer, int rlength, bool stop)
{
  sim_i2c_devaddr dad=m_dev_addrs[device];
  int ret;
  if ((ret = m_sim_devs.exchange_message(dad, wbuffer, wlength,
                                         rbuffer, rlength, stop) < 0))
   {
    return false;
   }

  return true;
}

bool
HW_V4::I2CRead(t_i2cdevices device, uint8_t* rbuffer, int rlength, bool stop)
{
  sim_i2c_devaddr dad=m_dev_addrs[device];
  int ret;
  if (ret = m_sim_devs.exchange_message(dad, NULL, 0,
                                        rbuffer, rlength, stop) < 0)
   {
    return false;
   }
  return true;
}

bool
HW_V4::PWMSet(hw_pwm id, float value)
{

  if ((value < 0) || (value > 100.0)) return false;

	switch (id)
	{
	case PWM_PV1:
		uint32_t v = (uint32_t)value * 4095.0 / 100.0;
		//ledcWrite(0, v);
		if (v > 0)
		//	digitalWrite(BREETHE, HIGH);
		break;

	}


  return true;
}

bool
HW_V4::IOSet(hw_gpio id, bool value)
{
	switch (id)
	{
	case GPIO_PV2:
		//digitalWrite(VALVE_OUT_PIN, value ? HIGH : LOW);
		if (value==LOW)
			//digitalWrite(BREETHE, LOW);
		break;
	case GPIO_BUZZER:
		//digitalWrite(BUZZER, value ? HIGH : LOW);
		break;
	case GPIO_LED:
		//digitalWrite(ALARM_LED, value ? HIGH : LOW);
		break;
	case GPIO_RELEALLARM:
		//digitalWrite(ALARM_RELE, value ? HIGH : LOW);
		break;
	default:
		return false;
		break;
	}
  return true;
}

bool
HW_V4::IOGet(hw_gpio id, bool* value)
{
	switch (id)
	{
	case GPIO_PV2:
		//*value = digitalRead(VALVE_OUT_PIN);
		break;
	case GPIO_BUZZER:
		//*value = digitalRead(BUZZER);
		break;
	case GPIO_LED:
		//*value = digitalRead(ALARM_LED);
		break;
	case GPIO_RELEALLARM:
		//*value = digitalRead(ALARM_RELE);
		break;
	default:
		return false;
		break;
	}
	return true;
}

void
HW_V4::__delay_blocking_ms(uint32_t ms)
{
	delay(ms);
}

void
HW_V4::PrintDebugConsole(String s)
{
}

void HW_V4::PrintLineDebugConsole(String s)
{
}

void HW_V4::Tick()
{
	if (Get_dT_millis(batteryStatus_reading_LT)>1000)
	{
		batteryStatus_reading_LT = GetMillis();
		currentBatteryCharge = 0; /* XXX */
		pWall = false; /* XXX */
		pIN = 0; /* XXX */
		BoardTemperature = 30; /* XXX */
		HW_AlarmsFlags = (uint16_t)0; /* XXX */

		//reset supervisor watchdog
 		//WriteSupervisor(0x00, 0);
		//Serial.println("Battery: " + String(currentBatteryCharge) + " PWALL: " + String (pWall));
	}
	

	return;
}
void HW_V4::GetPowerStatus(bool* batteryPowered, float* charge)
{
	*batteryPowered = pWall ? false:true;
	*charge = currentBatteryCharge ;

}



bool HW_V4::DataAvailableOnUART0()
{
	return false;
}

bool HW_V4::WriteUART0(String s)
{
	return true;
}
String HW_V4::ReadUART0UntilEOL()
{
	//PERICOLO. SE IL \n NON VIENE INVIATO TUTTO STALLA!!!!
	//return Serial.readStringUntil('\n');
        return String("");
}

uint64_t HW_V4::GetMillis()
{
	return (uint64_t)millis();
}
int64_t HW_V4::Get_dT_millis(uint64_t ms)
{
	return (int64_t)(millis() - ms);
}


void HW_V4::__service_i2c_detect()
{
  /* No state change ? Just a diagnostics printout, it would seem. */
  uint8_t error, address;
  int nDevices = 0;
  Serial.println("Scanning... I2C");
  nDevices = 0;
  for (address = 1; address < 127; address++)
   {
    sim_i2c_devaddr dad(current_muxpos, address);
    if (m_sim_devs.alive(dad))
     {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
       {
        Serial.print("0");
       }
      Serial.println(address, HEX);
      nDevices++;
     }
   }

  if (nDevices == 0)
   {
    Serial.println("No I2C devices found\n");
   }
}


void HW_V4::i2c_MuxSelect(uint8_t i)
{
	if (i > 7)
		return;

	if (i < 0)
		return;

	if (current_muxpos == i) return;

	current_muxpos = i;
}

t_i2cdev HW_V4::GetIICDevice(t_i2cdevices device)
{
	for (int i = 0; i < IIC_COUNT; i++)
	{
		if (iic_devs[i].t_device == device)
		{
			return iic_devs[i];
		}
	}
   t_i2cdev ret = { IIC_INVALID, 0, 0};
   return ret;
}



uint16_t HW_V4::ReadSupervisor(uint8_t i_address)
{
	uint8_t wbuffer[4];
	uint8_t rbuffer[4];
        uint16_t a;
		
	wbuffer[0] = i_address;

	a = (rbuffer [1]<< 8) | rbuffer[0];
	return a;
}


void HW_V4::WriteSupervisor( uint8_t i_address, uint16_t write_data)
{
	uint8_t wbuffer[4];
	wbuffer[0] = i_address;
	wbuffer[1] = write_data & 0xFF;
	wbuffer[2] = (write_data >> 8) & 0xFF;
}

float HW_V4::GetPIN()
{
	return pIN;
}

float HW_V4::GetBoardTemperature()
{
	return BoardTemperature;
}

uint16_t HW_V4::GetSupervisorAlarms()
{
	return HW_AlarmsFlags;
}

#include <sstream>
#include <iomanip>
#include "Serial.h"

bool
SerialImpl::available() { return true; }

void
SerialImpl::begin(unsigned long baud, uint32_t config, int8_t rxPin,
             int8_t txPin, bool invert, unsigned long timeout_ms)
{}

size_t
SerialImpl::println(const String &str)
{
  return println(str.c_str());
}

size_t
SerialImpl::println(const char str[])
{
  std::streampos before(m_ttys.tellp());
  m_ttys << str << std::endl << std::flush; 
  std::streampos after(m_ttys.tellp());
  if(m_ttys.good()) return (after - before);
  return -1;
}

size_t
SerialImpl::println(unsigned long val, int base)
{
  std::ostringstream ostr;
  ostr << std::setbase(base) << val;
  return println(ostr.str().c_str());
}

size_t
SerialImpl::print(const String &str)
{
  return print(str.c_str());
}

size_t
SerialImpl::print(const char str[])
{
  std::streampos before(m_ttys.tellp());
  m_ttys << str << std::endl << std::flush; 
  std::streampos after(m_ttys.tellp());
  if(m_ttys.good()) return (after - before);
  return -1;
}

String
SerialImpl::readStringUntil(char end)
{
  std::string result;
  std::getline(m_ttys, result, end);
  if(m_ttys.good()) return String(result.c_str());
  return String("");
}
