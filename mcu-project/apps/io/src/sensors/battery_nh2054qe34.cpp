#include "battery_nh2054qe34.h"

#include <nh2054qe34_driver.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

const struct device *battery_dev = DEVICE_DT_GET(DT_NODELABEL(smart_battery));

BatteryNh2054qe34::BatteryNh2054qe34(Logger &logger) : logger_{logger}
{
    if (!device_is_ready(battery_dev)) {
        logger_.err("NH2054QE34 battery not found. Aborting...");
    }

    return;
}

int BatteryNh2054qe34::TriggerSampling()
{
    int ret = 0;
    ret = sensor_sample_fetch(battery_dev);

    if (ret != 0) {
        logger_.err("Could not fetch sample data.");
    }

    return ret;
}

int BatteryNh2054qe34::GetTemperature(float *temp)
{
    int ret = 0;
    struct sensor_value battery_temp;

    ret = sensor_channel_get(battery_dev, SENSOR_CHAN_GAUGE_TEMP, &battery_temp);
    if (ret != 0) {
        logger_.err("Failed to get battery temperature.");
        return ret;
    }

    *temp = (sensor_value_to_double(&battery_temp) / 10) - KELVIN_CELSIUS_DIFF;
    return ret;
}

int BatteryNh2054qe34::GetVoltage(float *volt)
{
    int ret = 0;
    struct sensor_value battery_volt;

    ret = sensor_channel_get(battery_dev, SENSOR_CHAN_VOLTAGE, &battery_volt);
    if (ret != 0) {
        logger_.err("Failed to get battery voltage.");
        return ret;
    }

    *volt = sensor_value_to_double(&battery_volt);
    return ret;
}

int BatteryNh2054qe34::GetCurrent(float *current)
{
    int ret = 0;
    struct sensor_value battery_current;

    ret = sensor_channel_get(battery_dev, SENSOR_CHAN_CURRENT, &battery_current);
    if (ret != 0) {
        logger_.err("Failed to get battery current.");
        return ret;
    }

    *current = sensor_value_to_double(&battery_current);
    return ret;
}

int BatteryNh2054qe34::GetRemCapacity(uint32_t *rem_cap)
{
    int ret = 0;
    struct sensor_value battery_rem_capacity;

    ret = sensor_channel_get(battery_dev, SENSOR_CHAN_GAUGE_REMAINING_CHARGE_CAPACITY,
                             &battery_rem_capacity);
    if (ret != 0) {
        logger_.err("Failed to get remaining battery capacity.");
        return ret;
    }

    *rem_cap = (uint32_t)battery_rem_capacity.val1;
    return ret;
}

int BatteryNh2054qe34::GetStatus(uint16_t *status)
{
    int ret = 0;
    struct sensor_value battery_status;

    ret = sensor_channel_get(battery_dev, (sensor_channel)SENSOR_CHAN_STATUS, &battery_status);
    if (ret != 0) {
        logger_.err("Failed to get battery status.");
        return ret;
    }

    *status = (uint16_t)battery_status.val1;
    return ret;
}

int BatteryNh2054qe34::GetRelativeStateOfCharge(uint8_t *relative_charge_state)
{
    int ret = 0;
    sensor_value battery_state_of_charge;

    ret = sensor_channel_get(battery_dev, SENSOR_CHAN_GAUGE_STATE_OF_CHARGE,
                             &battery_state_of_charge);
    if (ret != 0) {
        logger_.err("Failed to get battery relative state of charge info.");
        return ret;
    }

    *relative_charge_state = (uint8_t)battery_state_of_charge.val1;
    return ret;
}

int BatteryNh2054qe34::GetCycleCount(uint16_t *cycle_count)
{
    int ret = 0;
    sensor_value battery_cycle_count;

    ret = sensor_channel_get(battery_dev, SENSOR_CHAN_GAUGE_CYCLE_COUNT, &battery_cycle_count);
    if (ret != 0) {
        logger_.err("Failed to get battery cycle count.");
        return ret;
    }

    *cycle_count = (uint16_t)battery_cycle_count.val1;
    return ret;
}