#pragma once

#include <stdint.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>

#define MILLIVOLTS_PER_VOLT                     1000
#define MILLIAMPS_PER_AMP                       1000

#define BIT_MASK_CURRENT_SCALING                0xf0 // Bit mask for current scaling (IPScale) in SpecificationInfo().
#define BIT_MASK_VOLTAGE_SCALING                0x0f // Bit mask for voltage scaling (VScale) in SpecificationInfo().

#define BATTERY_PROPERTY_BYTES                  2

#define I2C_COMMAND_BATTERY_MODE                0x03
#define I2C_COMMAND_TEMPERATURE                 0x08
#define I2C_COMMAND_VOLTAGE                     0x09
#define I2C_COMMAND_RELATIVE_STATE_OF_CHARGE    0x0d
#define I2C_COMMAND_REM_CAPACITY                0x0f
#define I2C_COMMAND_CURRENT                     0x10
#define I2C_COMMAND_STATUS                      0x16
#define I2C_COMMAND_CYCLE_COUNT                 0x17
#define I2C_COMMAND_SPECIFICATION_INFO          0x1a

struct nh2054qe34_config
{
    const struct i2c_dt_spec i2c_spec;
};

struct nh2054qe34_data
{
    uint8_t mode[BATTERY_PROPERTY_BYTES];
    uint8_t temp[BATTERY_PROPERTY_BYTES];
    uint8_t current[BATTERY_PROPERTY_BYTES];
    uint8_t volt[BATTERY_PROPERTY_BYTES];
    uint8_t rem_cap[BATTERY_PROPERTY_BYTES];
    uint8_t status[BATTERY_PROPERTY_BYTES];
    uint8_t relative_charge_state[BATTERY_PROPERTY_BYTES];
    uint8_t cycle_count[BATTERY_PROPERTY_BYTES];
    uint8_t spec_info[BATTERY_PROPERTY_BYTES];
};

///
/// @enum battery_command_t
///
/// @brief I2C commands to read battery properties according to Smart Battery Specification.
///
typedef enum
{
    BATTERY_MODE = I2C_COMMAND_BATTERY_MODE,
    BATTERY_TEMPERATURE = I2C_COMMAND_TEMPERATURE,
    BATTERY_VOLTAGE = I2C_COMMAND_VOLTAGE,
    BATTERY_CURRENT = I2C_COMMAND_CURRENT,
    BATTERY_REM_CAPACITY = I2C_COMMAND_REM_CAPACITY,
    BATTERY_STATUS = I2C_COMMAND_STATUS,
    BATTERY_RELATIVE_STATE_OF_CHARGE = I2C_COMMAND_RELATIVE_STATE_OF_CHARGE,
    BATTERY_CYCLE_COUNT = I2C_COMMAND_CYCLE_COUNT,
    BATTERY_SPECIFICATION_INFO = I2C_COMMAND_SPECIFICATION_INFO,
} battery_command_t;

 
/** @brief Sensor specific channels of nh2054qe34. */
enum nh2054qe34_channel{
    /// @brief Battery status code.
    SENSOR_CHAN_STATUS = SENSOR_CHAN_PRIV_START,
    SENSOR_CHAN_SPECIFICATION_INFORMATION,
};