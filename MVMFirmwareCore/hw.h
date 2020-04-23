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
   
       
    
        virtual bool Init();
        virtual bool I2CWrite(t_i2cdevices device, uint8_t* wbuffer, int wlength, bool stop);
        virtual bool I2CRead(t_i2cdevices device, uint8_t* wbuffer, int wlength, uint8_t* rbuffer, int rlength, bool stop);
        virtual bool I2CRead(t_i2cdevices device, uint8_t* rbuffer, int rlength, bool stop);
        virtual bool PWMSet(hw_pwm id, float value);
        virtual bool IOSet(hw_gpio id, bool value);
        virtual bool IOGet(hw_gpio id, bool* value);
        virtual void __delay_blocking_ms(uint32_t ms);
        virtual void PrintDebugConsole(String s);
        virtual void PrintLineDebugConsole(String s);
        virtual void Tick();
        virtual uint64_t GetMillis();
        virtual int64_t Get_dT_millis(uint64_t ms);
        virtual bool DataAvailableOnUART0();
        virtual String ReadUART0UntilEOL();
        virtual bool WriteUART0(String s);
        void GetPowerStatus(bool* batteryPowered, float* charge);
        float GetPIN();
        float GetBoardTemperature();
        uint16_t GetSupervisorAlarms();
     
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
