// driver_SFM3019.h

#ifndef _DRIVER_SFM3019_h
#define _DRIVER_SFM3019_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "DriverContext.h"
typedef struct {
    uint8_t i2c_address;
    int16_t flow_scale;
    int16_t flow_offset;
} SfmConfig;


typedef enum {
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS0 = 0x3603,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS1 = 0x3608,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS2 = 0x3615,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS3 = 0x361E,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS4 = 0x3624,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS5 = 0x362F,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS_MIX_0 = 0x3632,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS_MIX_1 = 0x3639,
    SFM_CMD_START_CONTINUOUS_MEASUREMENT_GAS_MIX_2 = 0x3646,
} SfmCmdStartContinuousMeasurement;


class SensorSFM3019
{
public:


	bool Init(t_i2cdevices device, void* hw_handle);
    bool doMeasure(float* Flow, float* T);
    float GetIntegral();
    void ResetIntegral();

private:

	uint8_t i2c_address;
    t_i2cdevices i2c_device;


	HW* hwi;
	DebugIfaceClass* dbg;

    float Integral;
    bool _initialized = false;

    /**
     * Return the driver version
     * @return  Driver version string
     */
    const char* sfm_common_get_driver_version(void);

    /**
     * Detects if a sensor is connected by reading out the ID register.
     * If the sensor does not answer or if the answer is not the expected value,
     * the test fails.
     *
     * @param   i2c_address I2C address to probe
     *
     * @return 0 if a sensor was detected
     */
    int16_t sfm_common_probe(uint8_t i2c_address);

    /**
     * Read the product identifier, consisting of a product number and a serial
     * number.
     *
     * @param   i2c_address     I2C address to read the product identifier from
     * @param   product_number  Out parameter to store the product number
     * @param   serial_number   Out parameter to store the serial number in raw
     *                          byte format
     *
     * @return  0 on success, an error code otherwise
     */
    int16_t sfm_common_read_product_identifier(uint8_t i2c_address,
        uint32_t* product_number,
        uint8_t(*serial_number)[8]);

    /**
     * Read the scale factor, offset and unit for the given measurement type.
     *
     * @param   sfm_config      Pointer to the SFM object
     * @param   measurement_cmd Measurement type to get the scale, offset and unit
     * @param   flow_scale      Out parameter to store the flow scale
     * @param   flow_offset     Out parameter to store the flow offset
     * @param   unit            Out parameter to store the unit
     *
     * @return  0 on success, an error code otherwise
     */
    int16_t sfm_common_read_scale_factor_offset_and_unit(
        const SfmConfig* sfm_config,
        SfmCmdStartContinuousMeasurement measurement_cmd, int16_t* flow_scale,
        int16_t* flow_offset, uint16_t* unit);

    /**
     * Convert the raw flow ticks to slm.
     *
     * @param   sfm_config      Pointer to the SFM object
     * @param   flow_raw        Flow value as read by
     *                          sfm_common_read_measurement_raw
     * @param   flow            Out parameter to store the converted flow
     *
     * @return  0 on success, an error code otherwise
     */
    int16_t sfm_common_convert_flow_float(const SfmConfig* sfm_config,
        int16_t flow_raw, float* flow);

    /**
     * Convert the raw temperature ticks to degree Celsius.
     *
     * @param   temperature_raw Temperature value as read by
     *                          sfm_common_read_measurement_raw
     *
     * @return  The temperature in degree Celsius
     */
    float sfm_common_convert_temperature_float(int16_t temperature_raw);

    /**
     * Starts a continuous measurement with the given gas configuration.
     *
     * @param   sfm_config      Pointer to the SFM object
     * @param   measurement_cmd Select which gas or gas mix should be measured
     *
     * @return  0 on success, an error code otherwise
     */
    int16_t sfm_common_start_continuous_measurement(
        SfmConfig* sfm_config, SfmCmdStartContinuousMeasurement measurement_cmd);

    /**
     * Read results of a continuous measurement
     *
     * @param   sfm_config      Pointer to the SFM object
     * @param   flow_raw        Out parameter for the raw flow value
     * @param   temperature_raw Out parameter for the raw temperature value
     * @param   status          Out parameter for the status word
     *
     * @return  0 on success, an error code otherwise
     */
    int16_t sfm_common_read_measurement_raw(const SfmConfig* sfm_config,
        int16_t* flow_raw,
        int16_t* temperature_raw,
        uint16_t* status);

    /**
     * Stops a continuous measurement.
     *
     * @param   sfm_config      Pointer to the SFM object
     *
     * @return  0 on success, an error code otherwise
     */
    int16_t sfm_common_stop_continuous_measurement(SfmConfig* sfm_config);


    uint8_t sensirion_common_generate_crc(uint8_t* data, uint16_t count);

    int8_t sensirion_common_check_crc(uint8_t* data, uint16_t count,
        uint8_t checksum);

    /**
     * Send a general call reset.
     *
     * @warning This will reset all attached I2C devices on the bus which support
     *          general call reset.
     *
     * @return  STATUS_OK on success, an error code otherwise
     */
    int16_t sensirion_i2c_general_call_reset(void);

    /**
     * sensirion_fill_cmd_send_buf() - create the i2c send buffer for a command and
     *                                 a set of argument words. The output buffer
     *                                 interleaves argument words with their
     *                                 checksums.
     * @buf:        The generated buffer to send over i2c. Then buffer length must
     *              be at least SENSIRION_COMMAND_LEN + num_args *
     *              (SENSIRION_WORD_SIZE + CRC8_LEN).
     * @cmd:        The i2c command to send. It already includes a checksum.
     * @args:       The arguments to the command. Can be NULL if none.
     * @num_args:   The number of word arguments in args.
     *
     * @return      The number of bytes written to buf
     */
    uint16_t sensirion_fill_cmd_send_buf(uint8_t* buf, uint16_t cmd,
        const uint16_t* args, uint8_t num_args);

    /**
     * sensirion_i2c_read_words() - read data words from sensor
     *
     * @address:    Sensor i2c address
     * @data_words: Allocated buffer to store the read words.
     *              The buffer may also have been modified on STATUS_FAIL return.
     * @num_words:  Number of data words to read (without CRC bytes)
     *
     * @return      STATUS_OK on success, an error code otherwise
     */
    int16_t sensirion_i2c_read_words(uint8_t address, uint16_t* data_words,
        uint16_t num_words);

    /**
     * sensirion_i2c_read_words_as_bytes() - read data words as byte-stream from
     *                                       sensor
     *
     * Read bytes without adjusting values to the uP's word-order.
     *
     * @address:    Sensor i2c address
     * @data:       Allocated buffer to store the read bytes.
     *              The buffer may also have been modified on STATUS_FAIL return.
     * @num_words:  Number of data words(!) to read (without CRC bytes)
     *              Since only word-chunks can be read from the sensor the size
     *              is still specified in sensor-words (num_words = num_bytes *
     *              SENSIRION_WORD_SIZE)
     *
     * @return      STATUS_OK on success, an error code otherwise
     */
    int16_t sensirion_i2c_read_words_as_bytes(uint8_t address, uint8_t* data,
        uint16_t num_words);

    /**
     * sensirion_i2c_write_cmd() - writes a command to the sensor
     * @address:    Sensor i2c address
     * @command:    Sensor command
     *
     * @return      STATUS_OK on success, an error code otherwise
     */
    int16_t sensirion_i2c_write_cmd(uint8_t address, uint16_t command);

    /**
     * sensirion_i2c_write_cmd_with_args() - writes a command with arguments to the
     *                                       sensor
     * @address:    Sensor i2c address
     * @command:    Sensor command
     * @data:       Argument buffer with words to send
     * @num_words:  Number of data words to send (without CRC bytes)
     *
     * @return      STATUS_OK on success, an error code otherwise
     */
    int16_t sensirion_i2c_write_cmd_with_args(uint8_t address, uint16_t command,
        const uint16_t* data_words,
        uint16_t num_words);

    /**
     * sensirion_i2c_delayed_read_cmd() - send a command, wait for the sensor to
     *                                    process and read data back
     * @address:    Sensor i2c address
     * @cmd:        Command
     * @delay:      Time in microseconds to delay sending the read request
     * @data_words: Allocated buffer to store the read data
     * @num_words:  Data words to read (without CRC bytes)
     *
     * @return      STATUS_OK on success, an error code otherwise
     */
    int16_t sensirion_i2c_delayed_read_cmd(uint8_t address, uint16_t cmd,
        uint32_t delay_us, uint16_t* data_words,
        uint16_t num_words);
    /**
     * sensirion_i2c_read_cmd() - reads data words from the sensor after a command
     *                            is issued
     * @address:    Sensor i2c address
     * @cmd:        Command
     * @data_words: Allocated buffer to store the read data
     * @num_words:  Data words to read (without CRC bytes)
     *
     * @return      STATUS_OK on success, an error code otherwise
     */
    int16_t sensirion_i2c_read_cmd(uint8_t address, uint16_t cmd,
        uint16_t* data_words, uint16_t num_words);


        /**
    * Detects if a sensor is connected by reading out the ID register.
    * If the sensor does not answer or if the answer is not the expected value,
    * the test fails.
    *
    * @return 0 if a sensor was detected
    */
    int16_t sfm3019_probe(void);

    /**
     * Create a new SFM3019 instance
     */
    SfmConfig sfm3019_create(void);



    /**
     * Select the current i2c bus by index.
     * All following i2c operations will be directed at that bus.
     *
     * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
     * bus)
     *
     * @param bus_idx   Bus index to select
     * @returns         0 on success, an error code otherwise
     */
    int16_t sensirion_i2c_select_bus(uint8_t bus_idx);

    /**
     * Initialize all hard- and software components that are needed for the I2C
     * communication.
     */
    void sensirion_i2c_init(void);

    /**
     * Release all resources initialized by sensirion_i2c_init().
     */
    void sensirion_i2c_release(void);

    /**
     * Execute one read transaction on the I2C bus, reading a given number of bytes.
     * If the device does not acknowledge the read command, an error shall be
     * returned.
     *
     * @param address 7-bit I2C address to read from
     * @param data    pointer to the buffer where the data is to be stored
     * @param count   number of bytes to read from I2C and store in the buffer
     * @returns 0 on success, error code otherwise
     */
    int8_t sensirion_i2c_read(uint8_t address, uint8_t* data, uint16_t count);

    /**
     * Execute one write transaction on the I2C bus, sending a given number of
     * bytes. The bytes in the supplied buffer must be sent to the given address. If
     * the slave device does not acknowledge any of the bytes, an error shall be
     * returned.
     *
     * @param address 7-bit I2C address to write to
     * @param data    pointer to the buffer containing the data to write
     * @param count   number of bytes to read from the buffer and send over I2C
     * @returns 0 on success, error code otherwise
     */
    int8_t sensirion_i2c_write(uint8_t address, const uint8_t* data,
        uint16_t count);

    /**
     * Sleep for a given number of microseconds. The function should delay the
     * execution approximately, but no less than, the given time.
     *
     * When using hardware i2c:
     * Despite the unit, a <10 millisecond precision is sufficient.
     *
     * When using software i2c:
     * The precision needed depends on the desired i2c frequency, i.e. should be
     * exact to about half a clock cycle (defined in
     * `SENSIRION_I2C_CLOCK_PERIOD_USEC` in `sensirion_arch_config.h`).
     *
     * Example with 400kHz requires a precision of 1 / (2 * 400kHz) == 1.25usec.
     *
     * @param useconds the sleep time in microseconds
     */
    void sensirion_sleep_usec(uint32_t useconds);


 
    SfmConfig sfm3019;

};


#endif

