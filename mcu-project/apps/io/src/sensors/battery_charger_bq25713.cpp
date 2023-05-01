#include "battery_charger_bq25713.h"

#include <bq25713_driver.h>
#include <zephyr/drivers/sensor.h>

#include <bitset>

BatteryChargerBq25713::BatteryChargerBq25713(Logger &logger) : logger_{logger}
{
    if (!device_is_ready(charger_dev)) {
        logger_.err("BQ25713 battery charger not found. Aborting...");
    }
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

int BatteryChargerBq25713::SetChargingConfig(int32_t chg_current, int32_t chg_volt,
                                             bool &current_set_success, bool &volt_set_success)
{
    int ret = 0;
    struct sensor_value val[] = {{chg_current, 0}, {chg_volt, 0}};

    ret = sensor_attr_set(charger_dev, (sensor_channel)SENSOR_CHAN_CHARGING_CONFIG,
                          SENSOR_ATTR_CONFIGURATION, val);

    std::bitset<CHAR_BIT * sizeof(int)> code(ret);
    if (code.test(ERR_CHARGING_CURRENT_BIT_POS)) {
        logger_.wrn("Failed to set battery charge controller current!");
        current_set_success = false;
    } else {
        current_set_success = true;
    }
    if (code.test(ERR_CHARGING_VOLTAGE_BIT_POS)) {
        logger_.wrn("Failed to set battery charge controller voltage!");
        volt_set_success = false;
    } else {
        volt_set_success = true;
    }

    return ret;
}

int BatteryChargerBq25713::GetChargerStatus(int32_t &chgr_status)
{
    int ret = 0;
    sensor_value val;

    ret |= sensor_sample_fetch(charger_dev);
    ret |= sensor_channel_get(charger_dev, (sensor_channel)SENSOR_CHAN_CHARGER_STATUS, &val);

    chgr_status = val.val1;
    return ret;
}