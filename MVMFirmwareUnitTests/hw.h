// hw.h

#ifndef _HW_h
#define _HW_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "generic_definitions.h"
#include <functional>


class HW {
public:
   
       
    
        virtual bool Init() = 0;
        virtual bool I2CWrite(t_i2cdevices device, uint8_t* wbuffer, int wlength, bool stop) = 0;
        virtual bool I2CRead(t_i2cdevices device, uint8_t* wbuffer, int wlength, uint8_t* rbuffer, int rlength, bool stop) = 0;
        virtual bool I2CRead(t_i2cdevices device, uint8_t* rbuffer, int rlength, bool stop) = 0;
        virtual bool PWMSet(hw_pwm id, float value) = 0;
        virtual bool IOSet(hw_gpio id, bool value) = 0;
        virtual bool IOGet(hw_gpio id, bool* value) = 0;
        virtual void __delay_blocking_ms(uint32_t ms) = 0;
        virtual void PrintDebugConsole(String s) = 0;
        virtual void PrintLineDebugConsole(String s) = 0;
        virtual void Tick() = 0;
        virtual uint64_t GetMillis() = 0;
        virtual int64_t Get_dT_millis(uint64_t ms) = 0;
        virtual bool DataAvailableOnUART0() = 0;
        virtual String ReadUART0UntilEOL() = 0;
        virtual bool WriteUART0(String s) = 0;
        virtual void GetPowerStatus(bool* batteryPowered, float* charge) = 0;
        virtual float GetPIN() = 0;
        virtual float GetBoardTemperature() = 0;
        virtual uint16_t GetSupervisorAlarms() = 0;
     
private:

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
