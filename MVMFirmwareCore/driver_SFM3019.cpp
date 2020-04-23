// 
// 
// 

#include "driver_SFM3019.h"




#define SENSIRION_BIG_ENDIAN 0
#define SFM3019_I2C_ADDRESS 0x2E

#define SFM3019_CMD_START_CONTINUOUS_MEASUREMENT_O2 \
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS0
#define SFM3019_CMD_START_CONTINUOUS_MEASUREMENT_AIR \
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS1
#define SFM3019_CMD_START_CONTINUOUS_MEASUREMENT_AIR_O2_MIX \
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS_MIX_0

#define SFM3019_SOFT_RESET_TIME_US 2000
#define SENSIRION_I2C_CLOCK_PERIOD_USEC 10


#define STATUS_OK 0
#define STATUS_FAIL (-1)

#if SENSIRION_BIG_ENDIAN
#define be16_to_cpu(s) (s)
#define be32_to_cpu(s) (s)
#define be64_to_cpu(s) (s)
#define SENSIRION_WORDS_TO_BYTES(a, w) ()

#else /* SENSIRION_BIG_ENDIAN */

#define be16_to_cpu(s) (((uint16_t)(s) << 8) | (0xff & ((uint16_t)(s)) >> 8))
#define be32_to_cpu(s)                                                         \
    (((uint32_t)be16_to_cpu(s) << 16) | (0xffff & (be16_to_cpu((s) >> 16))))
#define be64_to_cpu(s)                                                         \
    (((uint64_t)be32_to_cpu(s) << 32) |                                        \
     (0xffffffff & ((uint64_t)be32_to_cpu((s) >> 32))))
/**
 * Convert a word-array to a bytes-array, effectively reverting the
 * host-endianness to big-endian
 * @a:  word array to change (must be (uint16_t *) castable)
 * @w:  number of word-sized elements in the array (SENSIRION_NUM_WORDS(a)).
 */
#define SENSIRION_WORDS_TO_BYTES(a, w)                                         \
    for (uint16_t *__a = (uint16_t *)(a), __e = (w), __w = 0; __w < __e;       \
         ++__w) {                                                              \
        __a[__w] = be16_to_cpu(__a[__w]);                                      \
    }
#endif /* SENSIRION_BIG_ENDIAN */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF
#define CRC8_LEN 1

#define SENSIRION_COMMAND_SIZE 2
#define SENSIRION_WORD_SIZE 2
#define SENSIRION_NUM_WORDS(x) (sizeof(x) / SENSIRION_WORD_SIZE)
#define SENSIRION_MAX_BUFFER_WORDS 32


#define SFM_CMD_READ_PRODUCT_IDENTIFIER 0xE102

#define SFM_CMD_READ_SCALE_FACTOR_OFFSET_AND_FLOW_UNIT 0x3661

#define SFM_CMD_STOP_CONTINUOUS_MEASUREMENT 0x3FF9



const char* SFM_DRV_VERSION_STR = "0.1.0";


uint8_t SensorSFM3019::sensirion_common_generate_crc(uint8_t* data, uint16_t count) {
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;

    /* calculates 8-Bit checksum with given polynomial */
    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

int8_t SensorSFM3019::sensirion_common_check_crc(uint8_t* data, uint16_t count,
    uint8_t checksum) {
    if (sensirion_common_generate_crc(data, count) != checksum)
        return STATUS_FAIL;
    return STATUS_OK;
}

int16_t SensorSFM3019::sensirion_i2c_general_call_reset(void) {
    const uint8_t data = 0x06;
    hwi->I2CWrite(IIC_GENERAL_CALL_SENSIRION, (uint8_t *)&data, (uint16_t)sizeof(data), true);
    return 0;
}

uint16_t SensorSFM3019::sensirion_fill_cmd_send_buf(uint8_t* buf, uint16_t cmd,
    const uint16_t* args, uint8_t num_args) {
    uint8_t crc;
    uint8_t i;
    uint16_t idx = 0;

    buf[idx++] = (uint8_t)((cmd & 0xFF00) >> 8);
    buf[idx++] = (uint8_t)((cmd & 0x00FF) >> 0);

    for (i = 0; i < num_args; ++i) {
        buf[idx++] = (uint8_t)((args[i] & 0xFF00) >> 8);
        buf[idx++] = (uint8_t)((args[i] & 0x00FF) >> 0);

        crc = sensirion_common_generate_crc((uint8_t*)&buf[idx - 2],
            SENSIRION_WORD_SIZE);
        buf[idx++] = crc;
    }
    return idx;
}

int16_t SensorSFM3019::sensirion_i2c_read_words_as_bytes(uint8_t address, uint8_t* data,
    uint16_t num_words) {
    int16_t ret;
    uint16_t i, j;
    uint16_t size = num_words * (SENSIRION_WORD_SIZE + CRC8_LEN);
    uint16_t word_buf[SENSIRION_MAX_BUFFER_WORDS];
    uint8_t* const buf8 = (uint8_t*)word_buf;

    ret = sensirion_i2c_read(address, buf8, size);
    if (ret != STATUS_OK)
        return ret;

    /* check the CRC for each word */
    for (i = 0, j = 0; i < size; i += SENSIRION_WORD_SIZE + CRC8_LEN) {

        ret = sensirion_common_check_crc(&buf8[i], SENSIRION_WORD_SIZE,
            buf8[i + SENSIRION_WORD_SIZE]);
        if (ret != STATUS_OK)
            return ret;

        data[j++] = buf8[i];
        data[j++] = buf8[i + 1];
    }

    return STATUS_OK;
}

int16_t SensorSFM3019::sensirion_i2c_read_words(uint8_t address, uint16_t* data_words,
    uint16_t num_words) {
    int16_t ret;
    uint8_t i;

    ret = sensirion_i2c_read_words_as_bytes(address, (uint8_t*)data_words,
        num_words);
    if (ret != STATUS_OK)
        return ret;

    for (i = 0; i < num_words; ++i)
        data_words[i] = be16_to_cpu(data_words[i]);

    return STATUS_OK;
}

int16_t SensorSFM3019::sensirion_i2c_write_cmd(uint8_t address, uint16_t command) {
    uint8_t buf[SENSIRION_COMMAND_SIZE];

    sensirion_fill_cmd_send_buf(buf, command, NULL, 0);
    return sensirion_i2c_write(address, buf, SENSIRION_COMMAND_SIZE);
}

int16_t SensorSFM3019::sensirion_i2c_write_cmd_with_args(uint8_t address, uint16_t command,
    const uint16_t* data_words,
    uint16_t num_words) {
    uint8_t buf[SENSIRION_MAX_BUFFER_WORDS];
    uint16_t buf_size;

    buf_size = sensirion_fill_cmd_send_buf(buf, command, data_words, num_words);
    return sensirion_i2c_write(address, buf, buf_size);
}

int16_t SensorSFM3019::sensirion_i2c_delayed_read_cmd(uint8_t address, uint16_t cmd,
    uint32_t delay_us, uint16_t* data_words,
    uint16_t num_words) {
    int16_t ret;
    uint8_t buf[SENSIRION_COMMAND_SIZE];

    sensirion_fill_cmd_send_buf(buf, cmd, NULL, 0);
    ret = sensirion_i2c_write(address, buf, SENSIRION_COMMAND_SIZE);
    if (ret != STATUS_OK)
        return ret;

    if (delay_us)
        sensirion_sleep_usec(delay_us);

    return sensirion_i2c_read_words(address, data_words, num_words);
}

int16_t SensorSFM3019::sensirion_i2c_read_cmd(uint8_t address, uint16_t cmd,
    uint16_t* data_words, uint16_t num_words) {
    return sensirion_i2c_delayed_read_cmd(address, cmd, 0, data_words,
        num_words);
}




int8_t SensorSFM3019::sensirion_i2c_read(uint8_t address, uint8_t* data, uint16_t count) {
    uint8_t readData[32];
    uint8_t rxByteCount = 0;

    
    hwi->I2CRead(i2c_device, readData, count,true);
  
    memcpy(data, readData, count);

    return 0;
}

int8_t SensorSFM3019::sensirion_i2c_write(uint8_t address, const uint8_t* data,
    uint16_t count) {
  

    hwi->I2CWrite(i2c_device, (uint8_t*)data, count, true);


    return 0;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void SensorSFM3019::sensirion_sleep_usec(uint32_t useconds) {
    hwi->__delay_blocking_ms((useconds / 1000) + 1);
}


const char* SensorSFM3019::sfm_common_get_driver_version(void) {
    return SFM_DRV_VERSION_STR;
}

int16_t SensorSFM3019::sfm_common_probe(uint8_t i2c_address) {
    uint16_t buf[6];
    return sensirion_i2c_read_cmd(i2c_address, SFM_CMD_READ_PRODUCT_IDENTIFIER,
        buf, 6);
}

int16_t SensorSFM3019::sfm_common_read_product_identifier(uint8_t i2c_address,
    uint32_t* product_number,
    uint8_t(*serial_number)[8]) {
    uint8_t buf[6 * 2];
    int16_t error =
        sensirion_i2c_write_cmd(i2c_address, SFM_CMD_READ_PRODUCT_IDENTIFIER);
    if (error) {
        return error;
    }
    error = sensirion_i2c_read_words_as_bytes(i2c_address, buf, 6);
    if (error) {
        return error;
    }
    if (product_number) {
        *product_number = ((uint32_t)buf[0] << 24) + ((uint32_t)buf[1] << 16) +
            ((uint32_t)buf[2] << 8) + (uint32_t)buf[3];
    }
    if (serial_number) {
        for (size_t i = 0; i < 8; ++i) {
            (*serial_number)[i] = buf[i + 4];
        }
    }
    return 0;
}

int16_t SensorSFM3019::sfm_common_read_scale_factor_offset_and_unit(
    const SfmConfig* sfm_config,
    SfmCmdStartContinuousMeasurement measurement_cmd, int16_t* flow_scale,
    int16_t* flow_offset, uint16_t* unit) {

    uint16_t measurement_cmd_word = (uint16_t)measurement_cmd;

    uint16_t buf[3];
    int16_t error = sensirion_i2c_write_cmd_with_args(
        sfm_config->i2c_address, SFM_CMD_READ_SCALE_FACTOR_OFFSET_AND_FLOW_UNIT,
        &measurement_cmd_word, 1);
    if (error) {
        return error;
    }
    error =
        sensirion_i2c_read_words(sfm_config->i2c_address, buf, ARRAY_SIZE(buf));

    if (error) {
        return error;
    }
    if (flow_scale) {
        *flow_scale = (int16_t)buf[0];
    }
    if (flow_offset) {
        *flow_offset = (int16_t)buf[1];
    }
    if (unit) {
        *unit = buf[2];
    }
    return 0;
}

int16_t SensorSFM3019::sfm_common_convert_flow_float(const SfmConfig* sfm_config,
    int16_t flow_raw, float* flow) {
    if (sfm_config->flow_scale == 0) {
        return -1;
    }

    *flow =
        (flow_raw - sfm_config->flow_offset) / (float)(sfm_config->flow_scale);

    return 0;
}

float SensorSFM3019::sfm_common_convert_temperature_float(int16_t temperature_raw) {
    return temperature_raw / 200.0f;
}

int16_t SensorSFM3019::sfm_common_start_continuous_measurement(
    SfmConfig* sfm_config, SfmCmdStartContinuousMeasurement measurement_cmd) {

    int16_t error = sfm_common_read_scale_factor_offset_and_unit(
        sfm_config, measurement_cmd, &sfm_config->flow_scale,
        &sfm_config->flow_offset, NULL);
    if (error) {
        return error;
    }

    return sensirion_i2c_write_cmd(sfm_config->i2c_address, measurement_cmd);
}

int16_t SensorSFM3019::sfm_common_read_measurement_raw(const SfmConfig* sfm_config,
    int16_t* flow_raw,
    int16_t* temperature_raw,
    uint16_t* status) {
    uint16_t buf[3] = {};
    int16_t error = sensirion_i2c_read_words(sfm_config->i2c_address, buf, 3);
    if (error) {
        return error;
    }
    if (flow_raw) {
        *flow_raw = (int16_t)buf[0];
    }
    if (temperature_raw) {
        *temperature_raw = (int16_t)buf[1];
    }
    if (status) {
        *status = buf[2];
    }
    return 0;
}

int16_t SensorSFM3019::sfm_common_stop_continuous_measurement(SfmConfig* sfm_config) {
    sfm_config->flow_scale = 0;
    sfm_config->flow_offset = 0;
    return sensirion_i2c_write_cmd(sfm_config->i2c_address,
        SFM_CMD_STOP_CONTINUOUS_MEASUREMENT);
}


int16_t SensorSFM3019::sfm3019_probe(void) {
    return sfm_common_probe(i2c_address);
}

SfmConfig SensorSFM3019::sfm3019_create(void) {
    SfmConfig sfm_config = {
        i2c_address,
        0,
        0,
    };
    return sfm_config;
}



bool SensorSFM3019::Init(t_i2cdevices device, void* hw_handle)
{

    DriverContext* dc;
    dc = (DriverContext*)hw_handle;
    hwi = (HW*)dc->hwi;
    dbg = (DebugIfaceClass*)dc->dbg;
    i2c_address = 0x2e;
    i2c_device = device;

    _initialized = false;
    const char* driver_version = sfm_common_get_driver_version();
    if (driver_version) {
        dbg->DbgPrint(DBG_KERNEL, DBG_INFO, "SFM driver version " + String(driver_version));
    }
    else {
        dbg->DbgPrint(DBG_KERNEL, DBG_ERROR, "SFM fatal: Getting driver version failed");
        return false;
    }

    int16_t error;
    
    /* Reset all I2C devices */
    error = sensirion_i2c_general_call_reset();
    if (error) {
        dbg->DbgPrint(DBG_KERNEL, DBG_ERROR, "General call reset failed");
        return false;
    }
    /* Wait for the SFM3019 to initialize */
    sensirion_sleep_usec(SFM3019_SOFT_RESET_TIME_US*4);

    uint32_t timeout = millis();
    while (sfm3019_probe()) {
        dbg->DbgPrint(DBG_KERNEL, DBG_ERROR, "SFM sensor probing failed");
        if (millis() - timeout > 1000)
            return false;
        hwi->__delay_blocking_ms(10);
    }

    uint32_t product_number = 0;
    uint8_t serial_number[8] = {};
    error = sfm_common_read_product_identifier(i2c_address,
        &product_number, &serial_number);
    if (error) {
        dbg->DbgPrint(DBG_KERNEL, DBG_ERROR, "Failed to read product identifier");
        return false;
    }
    else {
        dbg->DbgPrint(DBG_KERNEL, DBG_WARNING, "Product: " + String(product_number));
        for (size_t i = 0; i < 8; ++i) {
            dbg->DbgPrint(DBG_KERNEL, DBG_WARNING, String(serial_number[i]));
        }
    }

    hwi->__delay_blocking_ms(1000);
    sfm3019 = sfm3019_create();

    error = sfm_common_start_continuous_measurement(
        &sfm3019, SFM3019_CMD_START_CONTINUOUS_MEASUREMENT_AIR);
    if (error) {
        dbg->DbgPrint(DBG_KERNEL, DBG_ERROR, "Failed to start measurement");
    }

    /* Wait for the first measurement to be available. Wait for
     * SFM3019_MEASUREMENT_WARM_UP_TIME_US instead for more reliable results */
    hwi->__delay_blocking_ms(100);

    _initialized = true;
    return true;
}


bool SensorSFM3019::doMeasure(float* Flow, float* T)
{
    int16_t flow_raw;
    int16_t temperature_raw;
    uint16_t status;
    int16_t error;
    if (!_initialized) return false;

    error = sfm_common_read_measurement_raw(&sfm3019, &flow_raw,
        &temperature_raw, &status);
    if (error) {
        //Serial.println("Error while reading measurement");
        return false;
    }
    else {
        float flow;
        float temperature;
        error = sfm_common_convert_flow_float(&sfm3019, flow_raw, &flow);
        if (error) {
            //Serial.println("Error while converting flow");
            return false;
        }
        *T = sfm_common_convert_temperature_float(temperature_raw);
        *Flow = flow;
        Integral += flow;
        //Serial.println("Flow: " + String(flow)  + " flow_raw: " + String(flow_raw) + " T: " +String(temperature) + " Traw: " +String(temperature_raw) + " status: " + String(status));
    }
}

float SensorSFM3019::GetIntegral()
{
    return Integral;
}
void SensorSFM3019::ResetIntegral()
{
    Integral = 0;
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
