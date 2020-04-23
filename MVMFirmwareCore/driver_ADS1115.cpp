// 
// 
// 

#include "driver_ADS1115.h"


#include <Wire.h>


static uint8_t i2cread(void) {
#if ARDUINO >= 100
    return Wire.read();
#else
    return Wire.receive();
#endif
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
    @param x byte to write
*/
/**************************************************************************/
static void i2cwrite(uint8_t x) {
#if ARDUINO >= 100
    Wire.write((uint8_t)x);
#else
    Wire.send(x);
#endif
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
    @param reg register address to write to
    @param value value to write to register
*/
/**************************************************************************/
void ADC_ADS1115::writeRegister(uint8_t reg, uint16_t value) {
    bool bres = true;
    uint8_t wbuffer[4];
    
    wbuffer[0] = (uint8_t)reg;
    wbuffer[1] = (uint8_t)(value >> 8);
    wbuffer[2] = (uint8_t)(value & 0xFF);
    bres = hwi->I2CWrite(i2c_device, wbuffer, 3, true);
    

}

/**************************************************************************/
/*!
    @brief  Read 16-bits from the specified destination register
    @param reg register address to read from
    @return 16 bit register value read
*/
/**************************************************************************/
uint16_t ADC_ADS1115::readRegister(uint8_t reg) {
    bool bres = true;
    uint8_t wbuffer[2];
    uint8_t rbuffer[4];

    wbuffer[0] = (uint8_t)reg;
    bres = hwi->I2CRead(i2c_device, wbuffer, 1, rbuffer, 2, true);

    return ((rbuffer[0] << 8) | rbuffer[1]);
}


/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
    @param device I2C device
    @param handle handle to DriverContext struct
    @return true if success
*/
/**************************************************************************/
bool ADC_ADS1115::Init(t_i2cdevices device, void* handle){
    i2c_device = device;
    DriverContext* dc;
    dc = (DriverContext*)handle;
    hwi = (HW*)dc->hwi;
    dbg = (DebugIfaceClass*)dc->dbg;

    m_conversionDelay = ADS1115_CONVERSIONDELAY;
    m_bitShift = 0;
    m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */

    _initialized = true;

    return true;
}


/**************************************************************************/
/*!
    @brief  Sets the gain and input voltage range
    @param gain gain setting to use
*/
/**************************************************************************/
void ADC_ADS1115::setGain(adsGain_t gain) { m_gain = gain; }

/**************************************************************************/
/*!
    @brief  Gets a gain and input voltage range
    @return the gain setting
*/
/**************************************************************************/
adsGain_t ADC_ADS1115::getGain() { return m_gain; }

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel
    @param channel ADC channel to read
    @return the ADC reading
*/
/**************************************************************************/
uint16_t ADC_ADS1115::readADC_SingleEnded(uint8_t channel) {
    if (channel > 3) {
        return 0;
    }
    
    if (!_initialized) return false;

    // Start with default values
    uint16_t config =
        ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
        ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
        ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
        ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
        ADS1015_REG_CONFIG_DR_1600SPS |   // 1600 samples per second (default)
        ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

    // Set PGA/voltage range
    config |= m_gain;

    // Set single-ended input channel
    switch (channel) {
    case (0):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
        break;
    case (1):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
        break;
    case (2):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
        break;
    case (3):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
        break;
    }

    // Set 'start single-conversion' bit
    config |= ADS1015_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(ADS1015_REG_POINTER_CONFIG, config);

    // Wait for the conversion to complete
    delay(m_conversionDelay);

    // Read the conversion results
    // Shift 12-bit results right 4 bits for the ADS1015
    uint16_t value = readRegister(ADS1015_REG_POINTER_CONVERT) >> m_bitShift;

    dbg->DbgPrint(DBG_KERNEL, DBG_VALUE, "CHANNEL:  " + String(channel) + "   VALUE:  " + String(value));

    return value;
}


/**************************************************************************/
/*!
    @brief  Start a single-ended ADC reading from the specified channel
    @param channel ADC channel to read
    @return true if success, false if measure already im progress
*/
/**************************************************************************/
bool ADC_ADS1115::asyncMeasure(uint8_t channel) {
    if (channel > 3) {
        return 0;
    }

    if (!_initialized) return false;

    if (__pending_meas)
        return false;

    // Start with default values
    uint16_t config =
        ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
        ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
        ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
        ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
        ADS1015_REG_CONFIG_DR_1600SPS |   // 1600 samples per second (default)
        ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

    // Set PGA/voltage range
    config |= m_gain;

    // Set single-ended input channel
    switch (channel) {
    case (0):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
        break;
    case (1):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
        break;
    case (2):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
        break;
    case (3):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
        break;
    }

    // Set 'start single-conversion' bit
    config |= ADS1015_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(ADS1015_REG_POINTER_CONFIG, config);

    __last_millis = hwi->GetMillis();
    __pending_meas = true;
    return true;
}



/**************************************************************************/
/*!
    @brief Get result of ADC reading
    @param pointer to data to be read
    @return true if success, false if measure not in progress or read to early
*/
/**************************************************************************/

bool ADC_ADS1115::asyncGetResult(uint32_t* value)
{
    if (!_initialized) return false;

    if (!__pending_meas)
        return false;

    if (hwi->Get_dT_millis(__last_millis) < m_conversionDelay)
        return false;

    __pending_meas = false;

    *value = readRegister(ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
    return true;
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
