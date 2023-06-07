#include "battery_charger_bq25713.h"

#include <bq25713_driver.h>
#include <zephyr/drivers/sensor.h>

#include <bitset>

BatteryChargerBq25713::BatteryChargerBq25713(Logger &logger) : logger_{logger}
{}

int BatteryChargerBq25713::Init()
{
    logger_.inf("Battery Charger Bq25713 Init");

    if (!device_is_ready(charger_dev)) {
        logger_.err("BQ25713 battery charger not found. Aborting...");
        return 1;
    }

    return 0;
}

int BatteryChargerBq25713::SetChargingCurrent(int32_t chg_current)
{
    int ret = 0;
    struct sensor_value chg_current_ = {chg_current, 0};
    ret = sensor_attr_set(charger_dev, SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT,
                          SENSOR_ATTR_CONFIGURATION, &chg_current_);

    return ret;
}

int BatteryChargerBq25713::SetChargingVoltage(int32_t chg_volt)
{
    int ret = 0;
    struct sensor_value val = {chg_volt, 0};
    ret = sensor_attr_set(charger_dev, (sensor_channel)SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE,
                          SENSOR_ATTR_CONFIGURATION, &val);

    return ret;
}

int BatteryChargerBq25713::SetChargingConfig(int32_t chg_current, int32_t chg_volt)
{
    int ret = 0;
    struct sensor_value val[] = {{chg_volt, 0}, {chg_current, 0}};

    ret = sensor_attr_set(charger_dev, (sensor_channel)SENSOR_CHAN_CHARGING_CONFIG,
                          SENSOR_ATTR_CONFIGURATION, val);

    // Interpreting return codes
    std::bitset<CHAR_BIT * sizeof(int)> code(ret);
    if (code.test(ERR_CHARGING_VOLTAGE_BIT_POS)) {
        code.flip(ERR_CHARGING_VOLTAGE_BIT_POS);
        if (code == ERANGE) {
            return ERRNO_CHARGER_VOLTAGE_ERANGE;
        }
        return ERRNO_CHARGER_VOLTAGE_EIO;

    } else if (code.test(ERR_CHARGING_CURRENT_BIT_POS)) {
        code.flip(ERR_CHARGING_CURRENT_BIT_POS);
        if (code == ERANGE) {
            return ERRNO_CHARGER_CURRENT_ERANGE;
        }
        return ERRNO_CHARGER_CURRENT_EIO;

    } else {
        return (int)code.to_ulong();
    }
}

int BatteryChargerBq25713::GetChargerStatus(int32_t &chgr_status)
{
    int ret = 0;
    sensor_value val;

    ret = sensor_sample_fetch(charger_dev);
    if (ret != 0) {
        return ret;
    }
    ret = sensor_channel_get(charger_dev, (sensor_channel)SENSOR_CHAN_CHARGER_STATUS, &val);
    if (ret != 0) {
        return ret;
    }

    chgr_status = val.val1;
    return ret;
}