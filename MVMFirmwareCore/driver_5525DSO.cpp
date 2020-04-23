// 
// 
// 

#include "driver_5525DSO.h"



bool Sensor5525DSO::Init(t_i2cdevices device, t_ps_sensor model, t_ps_resolution ps_resolution, void* handle)
{
    DriverContext* dc;
    dc = (DriverContext*)handle;
    hwi = (HW*)dc->hwi;
    dbg = (DebugIfaceClass*)dc->dbg;
    i2c_device = device;
    sensor_model = model;
    sensor_resolution = ps_resolution;

    //Download calibration table from sensor
    _initialized = false;
    bool bres = true;
    uint8_t wbuffer[6];
    uint8_t rbuffer[6];
    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "Starting 5525DSO Initialization... ");

    Reset_5525DSO();
    hwi->__delay_blocking_ms(100);

    for (int i = 0; i < 6; i++) {
        wbuffer[0] = 0xA0 + ((i + 1) << 1);
        bres = hwi->I2CRead(i2c_device, wbuffer, 1, rbuffer, 2, true);
        if (!bres) return false;
        sensorCT.C[i] = (rbuffer[0] << 8) + rbuffer[1];
    }
    sensorCT.ZERO = 0;

    switch (sensor_model) {
        case DS_01:
            sensorCT.Q[0] = 15;
            sensorCT.Q[1] = 17;
            sensorCT.Q[2] = 7;
            sensorCT.Q[3] = 5;
            sensorCT.Q[4] = 7;
            sensorCT.Q[5] = 21;
            break;

        case GS_05:
            sensorCT.Q[0] = 16;
            sensorCT.Q[1] = 17;
            sensorCT.Q[2] = 6;
            sensorCT.Q[3] = 5;
            sensorCT.Q[4] = 7;
            sensorCT.Q[5] = 21;
            break;
    }

    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "SENS_T1:          " + String(sensorCT.C[0]));
    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "OFF_T1:           " + String(sensorCT.C[1]));
    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "TCS:              " + String(sensorCT.C[2]));
    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "TCO:              " + String(sensorCT.C[3]));
    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "TREF:             " + String(sensorCT.C[4]));
    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "TEMPSENS:         " + String(sensorCT.C[5]));

    uint32_t __chache_P;
    uint32_t __chache_T;
    int __TDiv;
    __last_is_T=false;
 

   
    __chache_P=0;
    __chache_T=0;
    __TDiv=0;
    __pending_meas = false;

    startup_counter = 1;
    data_valid = false;

    /*for (int i = 0; i < PBUFFER_SIZE;i++)
    {
        PBuffer[i] = 0;
    }*/
    _initialized = true;
    __last_millis = hwi->GetMillis();
    return true;
}
bool Sensor5525DSO::doMeasure(float* P, float* T)
{
    bool bres = true;
    uint8_t wbuffer[6];
    uint8_t rbuffer[6];
    int32_t pressure_raw;
    int32_t temperature_raw;

    if (!_initialized) return false;
    //Read raw pressure
    wbuffer[0] = GetResolutionByteCodePressure();
    bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
    if (!bres) return false;
    hwi->__delay_blocking_ms(GetResolutionDelay()*2);

    wbuffer[0] = 0x00;
    bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
    if (!bres) return false;

    hwi->__delay_blocking_ms(2);

    bres = hwi->I2CRead(i2c_device, rbuffer, 3, true);
    if (!bres) return false;
  
    pressure_raw = (rbuffer[0] << 16) + (rbuffer[1] << 8) + rbuffer[2];

    //Read raw temperature
    wbuffer[0] = GetResolutionByteCodeTemp();
    bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
    if (!bres) return false;
    hwi->__delay_blocking_ms(GetResolutionDelay()*2);

    wbuffer[0] = 0x00;
    bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
    if (!bres) return false;

    hwi->__delay_blocking_ms(2);

    bres = hwi->I2CRead(i2c_device, rbuffer, 3, true);
    if (!bres) return false;

    temperature_raw = (rbuffer[0] << 16) + (rbuffer[1] << 8) + rbuffer[2];

    CalibrateDate_5525DSO(temperature_raw, pressure_raw, T, P);

    dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "T:  " + String(*T) + "   P:  " + String(*P));


    return true;
}

bool Sensor5525DSO::asyncMeasure()
{
    bool bres = true;
    uint8_t wbuffer[6];

    if (!_initialized) return false;

    if (__pending_meas)
        return false;

    if (__TDiv <= 0)
    {
        __TDiv = 50;
        wbuffer[0] = GetResolutionByteCodeTemp();
        bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
        if (!bres) return false;
        __last_is_T = true;
    }
    else
    {
        __TDiv--;
        wbuffer[0] = GetResolutionByteCodePressure();
        bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
        if (!bres) return false;
        __last_is_T = false;
    }
    __last_millis = hwi->GetMillis();
    __pending_meas = true;
    return true;
}
bool Sensor5525DSO::asyncGetResult(float* P, float* T)
{
    bool bres = true;
    uint8_t wbuffer[6];
    uint8_t rbuffer[6];
    int32_t pressure_raw;
    int32_t temperature_raw;

    if (!_initialized) return false;

    if (!__pending_meas)
        return false;

    if (hwi->Get_dT_millis(__last_millis) < GetResolutionDelay())
        return false;

    __pending_meas = false;

    wbuffer[0] = 0x00;
    bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
    if (!bres) return false;
    hwi->__delay_blocking_ms(2);
    bres = hwi->I2CRead(i2c_device, rbuffer, 3, true);
    if (!bres) return false;

    if (__last_is_T)
    {
        temperature_raw = (rbuffer[0] << 16) + (rbuffer[1] << 8) + rbuffer[2];
        __chache_T = temperature_raw;
        CalibrateDate_5525DSO(__chache_T, __chache_P, T, P);
    }
    else
    {
        pressure_raw = (rbuffer[0] << 16) + (rbuffer[1] << 8) + rbuffer[2];
        __chache_P = pressure_raw;
        CalibrateDate_5525DSO(__chache_T, __chache_P, T, P);
    }

    
    if (startup_counter > 0)
    {
        startup_counter--;
    }
    else
    {
        data_valid = true;
    }
    
    return data_valid;
}
void Sensor5525DSO::CalibrateDate_5525DSO(int32_t raw_temp, int32_t raw_pressure, float* T, float* P)
{
    float PINSIDE, PINSIDE_ZERO;
    int32_t Q1 = sensorCT.Q[0];
    int32_t Q2 = sensorCT.Q[1];
    int32_t Q3 = sensorCT.Q[2];
    int32_t Q4 = sensorCT.Q[3];
    int32_t Q5 = sensorCT.Q[4];
    int32_t Q6 = sensorCT.Q[5];

    int32_t C1 = sensorCT.C[0];
    int32_t C2 = sensorCT.C[1];
    int32_t C3 = sensorCT.C[2];
    int32_t C4 = sensorCT.C[3];
    int32_t C5 = sensorCT.C[4];
    int32_t C6 = sensorCT.C[5];

    int32_t dT;
    int64_t OFF;
    int64_t SENS;

    int32_t Temp;
    int64_t Pres;

    dT = raw_temp - (C5 * pow(2, Q5));
    Temp = 2000 + ((dT * C6) / pow(2, Q6));
    OFF = (C2 * pow(2, Q2)) + ((C4 * dT) / (pow(2, Q4)));
    SENS = (C1 * pow(2, Q1)) + ((C3 * dT) / (pow(2, Q3)));
    Pres = (((raw_pressure * SENS) / (pow(2, 21))) - OFF) / (pow(2, 15));

    *T = ((float)Temp) / 100.0;
    PINSIDE = ((float)Pres) / 10000.0 * 68.9476;
    PINSIDE_ZERO = PINSIDE - sensorCT.ZERO;
   //     Serial.println("DBG; " + String(PINSIDE) + " " + String(sensorCT.ZERO) + " " + String(PINSIDE_ZERO));
    *P = PINSIDE_ZERO;
}
uint8_t Sensor5525DSO::GetResolutionByteCodeTemp()
{
    switch (sensor_model)
    {
    case OVS_256:
        return 0x50;
    case OVS_512:
        return 0x52;
    case OVS_1024:
        return 0x54;
    case OVS_2048:
        return 0x56;
    case OVS_4096:
        return 0x58;
    default:
        break;
    }
}

uint8_t Sensor5525DSO::GetResolutionByteCodePressure()
{
    switch (sensor_model)
    {
    case OVS_256:
        return 0x40;
    case OVS_512:
        return 0x42;
    case OVS_1024:
        return 0x44;
    case OVS_2048:
        return 0x46;
    case OVS_4096:
        return 0x48;
    default:
        break;
    }
}


uint32_t Sensor5525DSO::GetResolutionDelay()
{
    switch (sensor_model)
    {
    case OVS_256:
        return 1;
    case OVS_512:
        return 2;
    case OVS_1024:
        return 5;
    case OVS_2048:
        return 6;
    case OVS_4096:
        return 11;
    default:
        break;
    }
}

bool Sensor5525DSO::Reset_5525DSO()
{
    bool bres = true;
    uint8_t wbuffer[6];

    //Read raw pressure
    wbuffer[0] = 0x1E;
    bres = hwi->I2CWrite(i2c_device, wbuffer, 1, true);
    if (!bres) return false;

  
    return true;
}

/*
void Sensor5525DSO::UpdateBuffer(float P)
{
    for (int i = 0;i < PBUFFER_SIZE - 1;i++)
    {
        PBuffer[PBUFFER_SIZE - i] = PBuffer[PBUFFER_SIZE - i - 1];
    }
    PBuffer[0] = P;
}
*/
void Sensor5525DSO::setZero(float value)
{
    sensorCT.ZERO = value;
}

float Sensor5525DSO::doZero()
{
    float T, P;
    float value=0;
    float cnt = 0;
    sensorCT.ZERO = 0;
    doMeasure(&P, &T);

    for (int i = 0; i < 50; i++)
    {
        
        if (doMeasure(&P, &T))
        {
            value += P;
            cnt++;
        }

    }
    if (cnt > 0)
    {
        value = value / cnt;
    }
    sensorCT.ZERO = value;
    doMeasure(&P, &T);
    return value;
}

void Sensor5525DSO::correctZero(float value)
{
    sensorCT.ZERO += value;
}

float Sensor5525DSO::GetConversionDelay()
{
    return (float) GetResolutionDelay();
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
