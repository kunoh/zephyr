#pragma once

#include <stdint.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>

#define I2C_CMD_CHARGE_OPTION_0         0x01
#define I2C_CMD_CHARGER_STATUS          0x13
#define I2C_CMD_CHARGING_CURRENT        0x14
#define I2C_CMD_CHARGING_VOLTAGE        0x15

#define CHARGER_PROPERTY_BYTES          2

// #define CHARGER_VOLTAGE_MAX             
#define BQ25713_CURRENT_MIN             64      // mA
#define BQ25713_CURRENT_MAX             8128    // mA
#define BQ25713_VOLT_MIN                1024    // mV
#define BQ25713_4_S_VOLT_MAX            19200   // mV

#define BQ25713_CURRENT_REG_BIT_MASK    0x1fb0
#define BQ25713_VOLTAGE_REG_BIT_MASK    0x7ff8
#define BQ25713_WDT_BIT_MASK            0x60
#define BQ25713_WDT_5_SEC               0x20    // Min: 4 s, Nom: 5.5 s, Max: 7 s
#define BQ25713_WDT_88_SEC              0x40    // Min: 70 s, Nom: 88 s, Max: 105 s

#define ERR_CHARGING_CURRENT_BIT_POS    6
#define ERR_CHARGING_VOLTAGE_BIT_POS    7
#define ERR_CHARGING_CURRENT_CODE       BIT(ERR_CHARGING_CURRENT_BIT_POS)
#define ERR_CHARGING_VOLTAGE_CODE       BIT(ERR_CHARGING_VOLTAGE_BIT_POS)

struct bq25713_config
{
    const struct i2c_dt_spec i2c_spec;
};

struct bq25713_data
{
    uint8_t chg_status[2];
    uint8_t chg_charging_current[2];
    uint8_t chg_charging_volt[2];
};

/// @brief Sensor specific channels of nh2054qe34
enum bq25713_channel{
    SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE = SENSOR_CHAN_PRIV_START, // Desired charging voltage in mV. Only a charging current channel type exists in Zephyr's sensor API
    SENSOR_CHAN_CHARGER_STATUS,
    SENSOR_CHAN_CHARGING_CONFIG,
    SENSOR_CHAN_INIT_CONFIG,
};

///
/// @enum charger_command_t
///
/// @brief I2C commands to read battery properties according to Smart Battery Specification.
///
typedef enum
{
    CHARGE_OPTION_0 = I2C_CMD_CHARGE_OPTION_0,
    CHARGER_STATUS = I2C_CMD_CHARGER_STATUS,
    CHARGER_CHARGING_CURRENT = I2C_CMD_CHARGING_CURRENT,
    CHARGER_CHARGING_VOLTAGE = I2C_CMD_CHARGING_VOLTAGE,
} charger_command_t;