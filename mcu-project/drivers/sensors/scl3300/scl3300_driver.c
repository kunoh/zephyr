#include "scl3300_driver.h"

#define DT_DRV_COMPAT murata_scl3300

#define SPI_READ 0
#define SPI_WRITE 0x1 << 7



// Commands for inclinometer. With CRC byte.
#define READ_ACC_X 0x040000F7
#define READ_ACC_Y 0x080000FD
#define READ_ACC_Z 0x0C0000FB
#define READ_SELF_TEST_OUTPUT 0x100000E9
#define ENABLE_ANGLE_OUTPUT 0xB0001F6F
#define READ_ANG_X 0x240000C7
#define READ_ANG_Y 0x280000CD
#define READ_ANG_Z 0x2C0000CB
#define READ_TEMPERATURE 0x140000EF
#define READ_STATUS_SUMMARY 0x180000E5
#define READ_ERR_FLAG_1 0x1C0000E3
#define READ_ERR_FLAG_2 0x200000C1
#define READ_MODE_REG 0x340000DF
#define CHANGE_TO_MODE_1 0xB400001F
#define CHANGE_TO_MODE_2 0xB4000102
#define CHANGE_TO_MODE_3 0xB4000225
#define CHANGE_TO_MODE_4 0xB4000338
#define POWER_DOWN_MODE 0xB400046B
#define WAKE_UP_FROM_PWRDOWN 0xB400001F
#define SW_RESET 0xB4002098
#define READ_WHO_AM_I 0x40000091



/// Conversion constants for raw measurements to standard units.
#define MODE_1_ACC_CONVERSION_CONST 6000            // LSB/g = 6000
#define MODE_1_ANG_CONVERSION_CONST 182  			// LSB/deg = 105
#define MODE_2_ACC_CONVERSION_CONST 3000            // LSB/g = 3000
#define MODE_2_ANG_CONVERSION_CONST 182  			// LSB/deg = 52
#define MODE_3_ACC_CONVERSION_CONST 12000           // LSB/g = 12000
#define MODE_3_ANG_CONVERSION_CONST 182 			// LSB/deg = 182
#define MODE_4_ACC_CONVERSION_CONST 12000           // LSB/g = 12000
#define MODE_4_ANG_CONVERSION_CONST 182 			// LSB/deg = 182


// 8 bit CRC, used when calculting SPI frame CRC byte.
static uint8_t crc8(uint8_t bit_value, uint8_t crc)
{
    uint8_t temp;
    temp = (uint8_t)(crc & 0x80);
    if (bit_value == 0x01) {
        temp ^= 0x80;
    }
    crc <<= 1;
    if (temp > 0) {
        crc ^= 0x1D;
    }
    return crc;
}

/// 
/// @brief Calculate CRC byte for a SPI frame.
/// 
/// Calculate CRC for 24 MSB's of the 32 bit dword
/// (8 LSB's are the CRC field and are not included in CRC calculation)
/// @param SPI frame
/// @return CRC byte.
/// 
uint8_t calculate_crc(uint32_t data)
{
    uint8_t bit_index;
    uint8_t bit_value;
    uint8_t crc;
    crc = 0xFF;
    for (bit_index = 31; bit_index > 7; bit_index--) {
        bit_value = (uint8_t)((data >> bit_index) & 0x01);
        crc = crc8(bit_value, crc);
    }
    crc = (uint8_t)~crc;
    return crc;
}

/// 
/// @brief Helper function for converting raw acc data to struct sensor_value.
/// 
/// @param val A pointer to a sensor_value struct.
/// @param inp The raw swnsor value.
/// @return 0 if successful, error status codes otherwise.
/// 
static inline int acc_value_from_raw(struct sensor_value *val, int32_t inp, enum InclMode mode)
{
    int64_t converted = inp;

    switch (mode) {
        case SCL3300_CONFIG_MODE_1:
            converted = converted * SENSOR_G / MODE_1_ACC_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_2:
            converted = converted * SENSOR_G / MODE_2_ACC_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_3:
            converted = converted * SENSOR_G / MODE_3_ACC_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_4:
            converted = converted * SENSOR_G / MODE_4_ACC_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_PWR_DOWN:
            return INCL_ERR_DEVICE_IN_PWR_DOWN;
			break;

        default:
            return INCL_ERR_INVALID_ARGUMENT;
            break;
    }

    val->val1 = (int32_t)(converted / 1000000);
    val->val2 = (int32_t)(converted % 1000000);
    return 0;
}

/// 
/// @brief Helper function for converting raw angle data to struct sensor_value.
/// 
/// @param val A pointer to a sensor_value struct.
/// @param inp The raw swnsor value.
/// @return 0 if successful, negative errno code if failure.
/// 
static inline int angle_value_from_raw(struct sensor_value *val, int32_t inp, enum InclMode mode)
{
    int64_t converted = inp;

    switch (mode) {
        case SCL3300_CONFIG_MODE_1:
            converted = converted * 1000000 / MODE_1_ANG_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_2:
            converted = converted * 1000000 / MODE_2_ANG_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_3:
            converted = converted * 1000000 / MODE_3_ANG_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_4:
            converted = converted * 1000000 / MODE_4_ANG_CONVERSION_CONST;
            break;

        case SCL3300_CONFIG_MODE_PWR_DOWN:
            return INCL_ERR_DEVICE_IN_PWR_DOWN;
			break;

        default:
            return INCL_ERR_INVALID_ARGUMENT;
            break;
    }

    val->val1 = (int32_t)(converted / 1000000);
    val->val2 = (int32_t)(converted % 1000000);
    return 0;
}

/// 
/// @brief Send/recieve one frame (32bit)
/// 
/// @param spi_inst Devicetree identifier for the SPI instance.
/// @param txData Data to send. 4bytes.
/// @param rxData pointer to min 4byte array for holding recieved data.
/// @return 0 if succesfull, error code otherwise.
/// 
int trancieve_oneframe(const struct spi_dt_spec spi_inst, uint32_t txData, uint8_t *rxData)
{
    int status = 0;
    uint8_t crc = 0xFF;
    uint8_t rxCrc = 0xFF;
    uint32_t temp_data = 0;

    uint8_t *rxbuffer = rxData;
    uint8_t txbuffer[] = {0x00, 0x00, 0x00, 0x00};
    uint32_t txb[] = {txData, 0, 0, 0};

    txbuffer[3] = (txData & 0xFF000000) >> 24;
    txbuffer[2] = (txData & 0x00FF0000) >> 16;
    txbuffer[1] = (txData & 0x0000FF00) >> 8;
    txbuffer[0] = (txData & 0x000000FF);

    const struct spi_buf rxbuf = {.buf = rxbuffer, .len = 4};
    const struct spi_buf_set rx_buffer = {.buffers = &rxbuf, .count = 1};

    const struct spi_buf txbuf = {.buf = &txb, .len = 4};
    const struct spi_buf_set tx_buffer = {.buffers = &txbuf, .count = 1};

    if (!spi_is_ready_dt(&spi_inst)) {
        printk("SPI bus %s not ready!\r\n", spi_inst.bus->name);
        return INCL_ERR_SPI_NOT_RDY;
    }

    // SCL3300 requires fourth byte to be CRC byte.
    crc = calculate_crc(txData);
    txbuffer[3] = crc;

    status = spi_transceive(spi_inst.bus, &spi_inst.config, &tx_buffer, &rx_buffer);

    if (status != INCL_STATUS_OK) {
        return INCL_ERR_SPI_ERR | status << 8;
    }

    temp_data = rxbuffer[0] | rxbuffer[1] << 8 | rxbuffer[2] << 16 | rxbuffer[3] << 24;
    rxCrc = calculate_crc(temp_data);

    if (rxbuffer[0] != rxCrc) {
		status = INCL_ERR_INCORRECT_CRC;
		printk("SPI CRC check failed.\r\n");
    }
    // Check the return code from the SCL3300 for error bits.
    if ((rxbuffer[3] & 0x03) == 0x3) {
        status = INCL_ERR_DEVICE_REPORTS_ERR;
    }


    return status;
}

/// 
/// Set mode on SCL3300. The mode affect all channels.
/// 
int scl3300_attr_set_mode(const struct device *dev, enum sensor_attribute attr,
                          const struct sensor_value *val)
{
    const struct scl3300_config *config = dev->config;
    struct scl3300_data *data = dev->data;
    int status = 0;
    uint8_t rxbuffer[] = {0, 0, 0, 0};
    int mode = val->val1;
    
    switch (mode) {
        case SCL3300_CONFIG_MODE_1:
            status = trancieve_oneframe(config->spi, CHANGE_TO_MODE_1, rxbuffer);
            if (status == INCL_STATUS_OK) {
                data->incl_operating_mode = 1;
            }
            break;
        case SCL3300_CONFIG_MODE_2:
            status = trancieve_oneframe(config->spi, CHANGE_TO_MODE_2, rxbuffer);
            if (status == INCL_STATUS_OK) {
                data->incl_operating_mode = 2;
            }
            break;
        case SCL3300_CONFIG_MODE_3:
            status = trancieve_oneframe(config->spi, CHANGE_TO_MODE_3, rxbuffer);
            if (status == INCL_STATUS_OK) {
                data->incl_operating_mode = 3;
            }
            break;
        case SCL3300_CONFIG_MODE_4:
            status = trancieve_oneframe(config->spi, CHANGE_TO_MODE_4, rxbuffer);
            if (status == INCL_STATUS_OK) {
                data->incl_operating_mode = 4;
            }
            break;
        case SCL3300_CONFIG_MODE_PWR_DOWN:
            status = trancieve_oneframe(config->spi, POWER_DOWN_MODE, rxbuffer);
            break;
        default:
            status = INCL_ERR_INVALID_ARGUMENT;
    }

    return status;
}

/// 
/// @brief Sensor API specified attribute set function.
/// 
static int scl3300_attr_set(const struct device *dev, enum sensor_channel chan,
                            enum sensor_attribute attr, const struct sensor_value *val)
{
    int status = 0;

    switch (attr) {
        case SENSOR_ATTR_CONFIGURATION:
            // printk("setting config to: %d \r\n",val->val1);
            status = scl3300_attr_set_mode(dev, attr, val);
            break;
        default:
            status = INCL_ERR_INVALID_ARGUMENT;
    }

    return status;
}

/// 
/// @brief Sensor API specified attribute get function.
///
static int scl3300_attr_get(const struct device *dev, enum sensor_channel chan,
                            enum sensor_attribute attr, struct sensor_value *val)
{
	const struct scl3300_config *config = dev->config;
    // struct scl3300_data *data = dev->data;
    int status = 0;
    uint8_t rxbuffer[] = {0, 0, 0, 0};
     
	switch (attr) {
        case SENSOR_ATTR_CONFIGURATION:

			status |= trancieve_oneframe(config->spi, READ_MODE_REG, rxbuffer);
            status |= trancieve_oneframe(config->spi, READ_MODE_REG, rxbuffer);
			val->val1 = rxbuffer[1]; 	// LSB of reg contains operating mode.
            break;
        default:
            status = INCL_ERR_INVALID_ARGUMENT;
			break;
    }

    return status;
}

/// 
/// @brief Sensor API specified function. Triggers a collection of samples from sensor into driver data-structs.
/// 
/// @param dev Inclinometer device struct.
/// @param chan Channel or channels to fetch.
/// 
/// @return Status code. 0=Success.
/// 
static int scl3300_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    const struct scl3300_config *config = dev->config;
    struct scl3300_data *data = dev->data;
    int status = 1;
    int attempts = 0;

    uint8_t rxbuffer[] = {0, 0, 0, 0};
    int32_t acc_x_raw = 0;
    int32_t acc_y_raw = 0;
    int32_t acc_z_raw = 0;
    int32_t ang_x_raw = 0;
    int32_t ang_y_raw = 0;
    int32_t ang_z_raw = 0;

    while (status != INCL_STATUS_OK && attempts < 3) {
        if (!spi_is_ready_dt(&config->spi)) {
            printk("SPI bus %s not ready!", config->spi.bus->name);
            return INCL_ERR_SPI_NOT_RDY;
        }

        status = trancieve_oneframe(config->spi, READ_ACC_X, rxbuffer);
        status |= trancieve_oneframe(config->spi, READ_ACC_Y, rxbuffer);
        acc_x_raw = (int32_t)(((uint16_t)rxbuffer[2]) << 8 | rxbuffer[1]);
        status |= trancieve_oneframe(config->spi, READ_ACC_Z, rxbuffer);
        acc_y_raw = (int16_t)(((uint16_t)rxbuffer[2]) << 8 | rxbuffer[1]);
        status |= trancieve_oneframe(config->spi, READ_ANG_X, rxbuffer);
        acc_z_raw = (int16_t)(((uint16_t)rxbuffer[2]) << 8 | rxbuffer[1]);
        status |= trancieve_oneframe(config->spi, READ_ANG_Y, rxbuffer);
        ang_x_raw = (int16_t)(((uint16_t)rxbuffer[2]) << 8 | rxbuffer[1]);
        status |= trancieve_oneframe(config->spi, READ_ANG_Z, rxbuffer);
        ang_y_raw = (int16_t)(((uint16_t)rxbuffer[2]) << 8 | rxbuffer[1]);
        status |= trancieve_oneframe(config->spi, READ_STATUS_SUMMARY, rxbuffer);
        ang_z_raw = (int16_t)(((uint16_t)rxbuffer[2]) << 8 | rxbuffer[1]);

        attempts++;
    }

    status |= acc_value_from_raw(&(data->accX_mms), acc_x_raw, data->incl_operating_mode);
	status |= acc_value_from_raw(&(data->accY_mms), acc_y_raw, data->incl_operating_mode);
	status |= acc_value_from_raw(&(data->accZ_mms), acc_z_raw, data->incl_operating_mode);
	status |= angle_value_from_raw(&(data->angleX_cdeg), ang_x_raw, data->incl_operating_mode);
	status |= angle_value_from_raw(&(data->angleY_cdeg), ang_y_raw, data->incl_operating_mode);
	status |= angle_value_from_raw(&(data->angleZ_cdeg), ang_z_raw, data->incl_operating_mode);

    return status;
}

/**
 * @brief Get function for collecting sensor values.
 * 
 * @param dev Inclinometer device struct.
 * @param chan Channel or channels to get.
 * @param val pointer to senser value struct, or array of sensor value struct. MAKE SURE THIS ARRAY HAS ROOM FOR ALL REQUESTED VALUES! If in doubt make it 3 long.
 * @return int 0 for success. INCL_ERR_INVALID_ARGUMENT if given invalid channel argument.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
static int scl3300_channel_get(const struct device *dev, enum sensor_channel chan,
                               struct sensor_value *val)
{
    // const struct scl3300_config *config = dev->config;
    struct scl3300_data *data = dev->data;

    switch (chan) {
        case SENSOR_CHAN_ACCEL_X:
            val->val1 = data->accX_mms.val1;
            val->val2 = data->accX_mms.val2;
            break;

        case SENSOR_CHAN_ACCEL_Y:
            val->val1 = data->accY_mms.val1;
            val->val2 = data->accY_mms.val2;
            break;

        case SENSOR_CHAN_ACCEL_Z:
            val->val1 = data->accZ_mms.val1;
            val->val2 = data->accZ_mms.val2;
            break;

        case SENSOR_CHAN_ACCEL_XYZ:
            val[0].val1 = data->accX_mms.val1;
            val[0].val2 = data->accX_mms.val2;
            val[1].val1 = data->accY_mms.val1;
            val[1].val2 = data->accY_mms.val2;
            val[2].val1 = data->accZ_mms.val1;
            val[2].val2 = data->accZ_mms.val2;
            break;

        case SENSOR_CHAN_ANGLE_X:
            val->val1 = data->angleX_cdeg.val1;
            val->val2 = data->angleX_cdeg.val2;
            break;

        case SENSOR_CHAN_ANGLE_Y:
            val->val1 = data->angleY_cdeg.val1;
            val->val2 = data->angleY_cdeg.val2;
            break;

        case SENSOR_CHAN_ANGLE_Z:
            val->val1 = data->angleZ_cdeg.val1;
            val->val2 = data->angleZ_cdeg.val2;
            break;

        case SENSOR_CHAN_ANGLE_XYZ:
            val[0].val1 = data->angleX_cdeg.val1;
            val[0].val2 = data->angleX_cdeg.val2;
            val[1].val1 = data->angleY_cdeg.val1;
            val[1].val2 = data->angleY_cdeg.val2;
            val[2].val1 = data->angleZ_cdeg.val1;
            val[2].val2 = data->angleZ_cdeg.val2;
            break;

        default:
			return INCL_ERR_INVALID_ARGUMENT;
            break;
    }

    return 0;
}

#pragma GCC diagnostic pop

///
/// @brief API init function.
/// @param dev Inclinometer device struct.
/// @return Status code. 0=Success.
/// 
int scl3300_init(const struct device *dev)
{
    const struct scl3300_config *config = dev->config;
    struct scl3300_data *data = dev->data;
    int status = 0;
    uint32_t temp_data = 0;

    uint8_t rxbuffer[] = {0, 0, 0, 0};

    if (!spi_is_ready_dt(&config->spi)) {
        printk("SPI bus %s not ready!", config->spi.bus->name);
        return -EINVAL;
    }

    

    temp_data = WAKE_UP_FROM_PWRDOWN;
    trancieve_oneframe(config->spi, temp_data, rxbuffer);

    k_msleep(1);

    temp_data = SW_RESET;  
    trancieve_oneframe(config->spi, temp_data, rxbuffer);
    k_msleep(2);

    temp_data = CHANGE_TO_MODE_1;
    trancieve_oneframe(config->spi, temp_data, rxbuffer);
    data->incl_operating_mode = SCL3300_CONFIG_MODE_1;

    temp_data = ENABLE_ANGLE_OUTPUT;  
    trancieve_oneframe(config->spi, temp_data, rxbuffer);
    k_msleep(25);   // 25ms wait is required for "settling of signal path". The neæxt reads are only for verifying that setup has completed successfully, these could perhabs be moved to a first read.

    temp_data = READ_STATUS_SUMMARY;
    trancieve_oneframe(config->spi, temp_data, rxbuffer);
    trancieve_oneframe(config->spi, temp_data, rxbuffer);
    status = trancieve_oneframe(config->spi, temp_data, rxbuffer);
    printk("SCL3300 Init completed with status: %d \r\n", ( (uint16_t)rxbuffer[2] << 8) | rxbuffer[1] );

    return status;
}

// Structs and defines for device tree

static const struct sensor_driver_api scl3300_driver_api = {
    .attr_set = scl3300_attr_set,
    .attr_get = scl3300_attr_get,
    .sample_fetch = scl3300_sample_fetch,
    .channel_get = scl3300_channel_get,
};



// SPI_MODE_CPOL   SPI_MODE_CPHA
#define SCL3300_INIT(n)                                                                  \
    static const struct scl3300_config scl3300_config_##n = {                            \
        .spi = SPI_DT_SPEC_INST_GET(n, SPI_WORD_SET(32) | SPI_TRANSFER_MSB, 600),        \
        .spi_chipselect = DT_INST_REG_ADDR(n),                                           \
    };                                                                                   \
                                                                                         \
    static struct scl3300_data scl3300_data_##n;                                         \
                                                                                         \
    SENSOR_DEVICE_DT_INST_DEFINE(n, scl3300_init, NULL, &scl3300_data_##n, &scl3300_config_##n, \
                          POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &scl3300_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SCL3300_INIT)