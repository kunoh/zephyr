#pragma once

#include <stdint.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>

// General data
#define ERR_SPEC_INFO_BIT_POS   5
#define ERR_SPEC_INFO_CODE      BIT(ERR_SPEC_INFO_BIT_POS)
#define ERR_VOLTAGE_BIT_POS     6
#define ERR_VOLTAGE_CODE        BIT(ERR_VOLTAGE_BIT_POS)
#define ERR_CURRENT_BIT_POS     7
#define ERR_CURRENT_CODE        BIT(ERR_CURRENT_BIT_POS)
#define ERR_REM_CAP_BIT_POS     8
#define ERR_REM_CAP_CODE        BIT(ERR_REM_CAP_BIT_POS)
#define ERR_TEMP_BIT_POS        9
#define ERR_TEMP_CODE           BIT(ERR_TEMP_BIT_POS)
#define ERR_CYCLE_COUNT_BIT_POS 10
#define ERR_CYCLE_COUNT_CODE    BIT(ERR_CYCLE_COUNT_BIT_POS)

// Charging data
#define ERR_CHARGING_CURRENT_BIT_POS    5
#define ERR_CHARGING_CURRENT_CODE       BIT(ERR_CHARGING_CURRENT_BIT_POS)
#define ERR_CHARGING_VOLTAGE_BIT_POS    6
#define ERR_CHARGING_VOLTAGE_CODE       BIT(ERR_CHARGING_VOLTAGE_BIT_POS)
#define ERR_STATUS_BIT_POS              7
#define ERR_STATUS_CODE                 BIT(ERR_STATUS_BIT_POS)
#define ERR_REL_CHARGE_STATE_BIT_POS    8
#define ERR_REL_CHARGE_STATE_CODE       BIT(ERR_REL_CHARGE_STATE_BIT_POS)

#define MILLIVOLTS_PER_VOLT                     1000
#define MILLIAMPS_PER_AMP                       1000

#define BIT_MASK_CAPACITY_SCALING               0xf0 // Bit mask for capacity scaling (IPScale) in SpecificationInfo().
#define BIT_MASK_CURRENT_SCALING                0xf0 // Bit mask for current scaling (IPScale) in SpecificationInfo().
#define BIT_MASK_VOLTAGE_SCALING                0x0f // Bit mask for voltage scaling (VScale) in SpecificationInfo().

#define BATTERY_MODE_ALARM_BIT_POS              5
#define BATTERY_MODE_CHARGER_BIT_POS            6
#define BATTERY_MODE_CAPACITY_BIT_POS           7

#define BATTERY_PROPERTY_BYTES                  2

#define SMBUS_COMMAND_BATTERY_MODE              0x03
#define SMBUS_COMMAND_TEMPERATURE               0x08
#define SMBUS_COMMAND_VOLTAGE                   0x09
#define SMBUS_COMMAND_CURRENT                   0x10
#define SMBUS_COMMAND_RELATIVE_STATE_OF_CHARGE  0x0d
#define SMBUS_COMMAND_REM_CAPACITY              0x0f
#define SMBUS_COMMAND_CHARGING_CURRENT          0x14
#define SMBUS_COMMAND_CHARGING_VOLTAGE          0x15
#define SMBUS_COMMAND_STATUS                    0x16
#define SMBUS_COMMAND_CYCLE_COUNT               0x17
#define SMBUS_COMMAND_SPECIFICATION_INFO        0x1a

struct nh2054qe34_config
{
    const struct i2c_dt_spec i2c_spec;
};

struct nh2054qe34_data
{
    uint8_t mode[BATTERY_PROPERTY_BYTES];
    uint8_t temp[BATTERY_PROPERTY_BYTES];
    uint8_t volt[BATTERY_PROPERTY_BYTES];
    uint8_t current[BATTERY_PROPERTY_BYTES];
    uint8_t rem_cap[BATTERY_PROPERTY_BYTES];
    uint8_t charging_current[BATTERY_PROPERTY_BYTES];
    uint8_t charging_volt[BATTERY_PROPERTY_BYTES];
    uint8_t status[BATTERY_PROPERTY_BYTES];
    uint8_t relative_charge_state[BATTERY_PROPERTY_BYTES];
    uint8_t cycle_count[BATTERY_PROPERTY_BYTES];
    uint8_t spec_info[BATTERY_PROPERTY_BYTES];
};

///
/// @enum battery_command_t
///
/// @brief SMBUS commands to read battery properties according to Smart Battery Specification.
///
typedef enum
{
    BATTERY_MODE = SMBUS_COMMAND_BATTERY_MODE,
    BATTERY_TEMPERATURE = SMBUS_COMMAND_TEMPERATURE,
    BATTERY_VOLTAGE = SMBUS_COMMAND_VOLTAGE,
    BATTERY_CURRENT = SMBUS_COMMAND_CURRENT,
    BATTERY_REM_CAPACITY = SMBUS_COMMAND_REM_CAPACITY,
    BATTERY_CHARGING_CURRENT = SMBUS_COMMAND_CHARGING_CURRENT,
    BATTERY_CHARGING_VOLTAGE = SMBUS_COMMAND_CHARGING_VOLTAGE,
    BATTERY_STATUS = SMBUS_COMMAND_STATUS,
    BATTERY_RELATIVE_STATE_OF_CHARGE = SMBUS_COMMAND_RELATIVE_STATE_OF_CHARGE,
    BATTERY_CYCLE_COUNT = SMBUS_COMMAND_CYCLE_COUNT,
    BATTERY_SPECIFICATION_INFO = SMBUS_COMMAND_SPECIFICATION_INFO,
} battery_command_t;

 
/** @brief Sensor specific channels of nh2054qe34. */
enum nh2054qe34_channel{
    SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE = SENSOR_CHAN_PRIV_START, // Only a charging current type-flag exists in Zephyr's sensor API
    SENSOR_CHAN_STATUS,
    SENSOR_CHAN_BATTERY_GENERAL,
    SENSOR_CHAN_BATTERY_CHARGING,
    SENSOR_CHAN_INIT_CONFIG,
};