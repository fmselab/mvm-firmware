#pragma once

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


#include <stdint.h>

//#define HARDWARE_TARGET_PLATFORM_V3 1
#define HARDWARE_TARGET_PLATFORM_V4 1

typedef enum { PWM_PV1 } hw_pwm;
typedef enum { GPIO_PV2, GPIO_BUZZER, GPIO_LED, GPIO_RELEALLARM } hw_gpio;
typedef enum { PRESSURE_VALVE, PRESSURE_PATIENT } t_pressure_selector;
typedef enum { PS_LOOP, PS_PATIENT, PS_VENTURI } t_pressure_sensor;
typedef enum {  IIC_INVALID, 
				IIC_PS_0, 
				IIC_PS_1, 
				IIC_PS_2, 
				IIC_PS_3, 
				IIC_PS_4, 
				IIC_PS_5, 
				IIC_FLOW1, 
				IIC_FLOW2, 
				IIC_ADC_0,
				IIC_SUPERVISOR, 
				IIC_EEPROM, 
				IIC_MUX,
				IIC_GENERAL_CALL_SENSIRION} t_i2cdevices;
typedef struct
{
	t_i2cdevices t_device;
	uint8_t address;
	int8_t muxport; //USE -1 to indicate ANY
} t_i2cdev;


#define __ERROR_INPUT_PRESSURE_LOW 0
#define __ERROR_INPUT_PRESSURE_HIGH 1
#define __ERROR_INSIDE_PRESSURE_LOW 2
#define __ERROR_INSIDE_PRESSURE_HIGH 3
#define __ERROR_BATTERY_LOW 4
#define __ERROR_LEAKAGE 5
#define __ERROR_FULL_OCCLUSION 6
#define __ERROR_PARTIAL_OCCLUSION 7
#define __ERROR_APNEA 22
#define __ERROR_ALARM_PI 29
#define __ERROR_WDOG_PI 30
#define __ERROR_SYSTEM_FALIURE 31


//SAFETY LIMITS (mbar)
#define PV1_SAFETY_LIMIT	70
#define PV2_SAFETY_LIMIT	50

//ACCEPTED INPUT PRESSURE (mbar)
#define MIN_PIN 3000
#define MAX_PIN 4500

typedef enum {
	PRESSURE_DROP_INHALE,
	UNABLE_TO_READ_SENSOR_PRESSURE,
	UNABLE_TO_READ_SENSOR_FLUX,
	UNABLE_TO_READ_SENSOR_VENTURI,
	ALARM_COMPLETE_OCCLUSION,
	ALARM_PARTIAL_OCCLUSION,
	ALARM_PRESSURE_INSIDE_TOO_HIGH,
	ALARM_PRESSURE_INSIDE_TOO_LOW,
	ALARM_LEAKAGE,
	BATTERY_LOW,
	ALARM_PRESSURE_INPUT_TOO_LOW,
	ALARM_PRESSURE_INPUT_TOO_HIGH,
	ALARM_GUI_ALARM,
	ALARM_GUI_WDOG,
	ALARM_OVER_UNDER_VOLTAGE,
	ALARM_SUPERVISOR,
	ALARM_OVERTEMPERATURE,
	ALARM_APNEA,
	UNPREDICTABLE_CODE_EXECUTION

} t_ALARM;

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
