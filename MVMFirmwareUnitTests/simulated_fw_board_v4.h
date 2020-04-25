//
// File: simulated_fw_board.h
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 23-Apr-2020 Initial version.
//
// Description:
// Software simulation of a generic MVM firmware board.
//

#ifndef _FW_BOARD_SIMUL_h
#define _FW_BOARD_SIMUL_h

// Make sure we don't include the definition in the firmware code.
#define _FW_BOARD_NI_V4_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "hw.h"

#include "simulate_i2c_devices.h"

#include <map>

#define IIC_COUNT 8

#define PLOOP_MODEL     GS_05
#define PPATIENT_MODEL  DS_01
#define PVENTURI        DS_01

class HW_V4 : public HW {
public:


    bool Init();
    bool I2CWrite(t_i2cdevices device, uint8_t* wbuffer, int wlength, bool stop);
    bool I2CRead(t_i2cdevices device, uint8_t* wbuffer, int wlength, uint8_t* rbuffer, int rlength, bool stop);
    bool I2CRead(t_i2cdevices device, uint8_t* rbuffer, int rlength, bool stop);
    bool PWMSet(hw_pwm id, float value);
    bool IOSet(hw_gpio id, bool value);
    bool IOGet(hw_gpio id, bool* value);
    void PrintDebugConsole(String s);
    void PrintLineDebugConsole(String s);
    void __delay_blocking_ms(uint32_t ms);
    void Tick();
    uint64_t GetMillis();
    int64_t Get_dT_millis(uint64_t ms);
    bool DataAvailableOnUART0();
    String ReadUART0UntilEOL();
    bool WriteUART0(String s);
    void GetPowerStatus(bool* batteryPowered, float* charge);
    float GetPIN();
    float GetBoardTemperature();
    uint16_t GetSupervisorAlarms();

private:

    
    void __service_i2c_detect();
    void i2c_MuxSelect(uint8_t i);
    t_i2cdev GetIICDevice(t_i2cdevices device);

    uint16_t ReadSupervisor( uint8_t i_address);
    void WriteSupervisor(uint8_t i_address, uint16_t write_data);


    t_i2cdev iic_devs[IIC_COUNT];
    uint8_t current_muxpos = 10;

    uint64_t batteryStatus_reading_LT;
    float currentBatteryCharge;
    bool pWall;

    float pIN;
    float BoardTemperature;
    uint16_t HW_AlarmsFlags;

    std::map<t_i2cdevices, sim_i2c_devaddr> m_dev_addrs;
    simulate_i2c_devices m_sim_devs;
};

#define LOW               0x0
#define HIGH              0x1

#endif
