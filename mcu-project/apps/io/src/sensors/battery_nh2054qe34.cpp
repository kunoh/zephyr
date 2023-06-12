#include "battery_nh2054qe34.h"

#include <nh2054qe34_driver.h>

#include <bitset>

BatteryNh2054qe34::BatteryNh2054qe34(Logger &logger) : logger_{logger}
{}

int BatteryNh2054qe34::Init()
{
    logger_.inf("Battery Nh2054qe34 Init");

    if (!device_is_ready(battery_dev_)) {
        logger_.err("NH2054QE34 battery not found. Aborting...");
        return 1;
    }

    return 0;
}

int BatteryNh2054qe34::TriggerGeneralSampling()
{
    return sensor_sample_fetch_chan(battery_dev_, (sensor_channel)SENSOR_CHAN_BATTERY_GENERAL);
}

int BatteryNh2054qe34::TriggerChargingSampling()
{
    return sensor_sample_fetch_chan(battery_dev_, (sensor_channel)SENSOR_CHAN_BATTERY_CHARGING);
}

int BatteryNh2054qe34::GetGeneralData(BatteryGeneralData &data)
{
    int ret = 0;
    struct sensor_value bat_gen_data[NUM_GENERAL_DATA];

    ret =
        sensor_channel_get(battery_dev_, (sensor_channel)SENSOR_CHAN_BATTERY_GENERAL, bat_gen_data);

    data.temp = (sensor_value_to_double(bat_gen_data) / 10) - KELVIN_CELSIUS_DIFF;
    data.current = sensor_value_to_double(&bat_gen_data[1]);
    data.volt = sensor_value_to_double(&bat_gen_data[2]);
    data.remaining_capacity = bat_gen_data[3].val1;
    data.cycle_count = bat_gen_data[4].val1;
    return ret;
}

int BatteryNh2054qe34::GetChargingData(BatteryChargingData &data)
{
    int ret = 0;
    struct sensor_value bat_chg_data[NUM_CHARGING_DATA];

    ret = sensor_channel_get(battery_dev_, (sensor_channel)SENSOR_CHAN_BATTERY_CHARGING,
                             bat_chg_data);

    data.des_chg_current = bat_chg_data->val1;
    data.des_chg_volt = bat_chg_data[1].val1;
    data.status = bat_chg_data[2].val1;
    data.relative_charge_state = bat_chg_data[3].val1;

    return ret;
}

int BatteryNh2054qe34::GetTemperature(float &temp)
{
    int ret = 0;
    struct sensor_value battery_temp;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_TEMP, &battery_temp);
    if (ret != 0) {
        logger_.err("Failed to get battery temperature.");
        return ret;
    }

    temp = (sensor_value_to_double(&battery_temp) / 10) - KELVIN_CELSIUS_DIFF;
    return ret;
}

int BatteryNh2054qe34::GetVoltage(float &volt)
{
    int ret = 0;
    struct sensor_value battery_volt;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_VOLTAGE, &battery_volt);
    if (ret != 0) {
        logger_.err("Failed to get battery voltage.");
        return ret;
    }

    volt = sensor_value_to_double(&battery_volt);
    return ret;
}

int BatteryNh2054qe34::GetCurrent(float &current)
{
    int ret = 0;
    struct sensor_value battery_current;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_CURRENT, &battery_current);
    if (ret != 0) {
        logger_.err("Failed to get battery current.");
        return ret;
    }

    current = sensor_value_to_double(&battery_current);
    return ret;
}

int BatteryNh2054qe34::GetRemCapacity(int32_t &rem_cap)
{
    int ret = 0;
    struct sensor_value battery_rem_capacity;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_REMAINING_CHARGE_CAPACITY,
                             &battery_rem_capacity);
    if (ret != 0) {
        logger_.err("Failed to get remaining battery capacity.");
        return ret;
    }

    rem_cap = battery_rem_capacity.val1;
    return ret;
}

int BatteryNh2054qe34::GetRelativeStateOfCharge(int32_t &relative_charge_state)
{
    int ret = 0;
    sensor_value battery_state_of_charge;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_STATE_OF_CHARGE,
                             &battery_state_of_charge);
    if (ret != 0) {
        logger_.err("Failed to get battery relative state of charge info.");
        return ret;
    }

    relative_charge_state = battery_state_of_charge.val1;
    return ret;
}

int BatteryNh2054qe34::GetCycleCount(int32_t &cycle_count)
{
    int ret = 0;
    sensor_value battery_cycle_count;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_CYCLE_COUNT, &battery_cycle_count);
    if (ret != 0) {
        logger_.err("Failed to get battery cycle count.");
        return ret;
    }

    cycle_count = battery_cycle_count.val1;
    return ret;
}

int BatteryNh2054qe34::GetChargingCurrent(int32_t &charging_current)
{
    int ret = 0;
    struct sensor_value battery_charging_current;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT,
                             &battery_charging_current);
    if (ret != 0) {
        logger_.err("Failed to get the battery's desired charging current.");
        return ret;
    }

    charging_current = battery_charging_current.val1;
    return ret;
}

int BatteryNh2054qe34::GetChargingVoltage(int32_t &charging_volt)
{
    int ret = 0;
    struct sensor_value battery_charging_volt;

    ret =
        sensor_channel_get(battery_dev_, (sensor_channel)SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE,
                           &battery_charging_volt);
    if (ret != 0) {
        logger_.err("Failed to get the battery's desired charging voltage.");
        return ret;
    }

    charging_volt = battery_charging_volt.val1;
    return ret;
}

int BatteryNh2054qe34::GetStatus(int32_t &status)
{
    int ret = 0;
    struct sensor_value battery_status;

    ret = sensor_channel_get(battery_dev_, (sensor_channel)SENSOR_CHAN_STATUS, &battery_status);
    if (ret != 0) {
        logger_.err("Failed to get battery status.");
        return ret;
    }

    status = battery_status.val1;
    return ret;
}

bool BatteryNh2054qe34::CanBeCharged(int32_t status_code)
{
    std::bitset<CHAR_BIT * sizeof(int32_t)> code(status_code);
    return !(code.test(BATTERY_STATUS_OVER_CHARGED_BIT_POS) ||
             code.test(BATTERY_STATUS_TERMINATE_CHARGE_BIT_POS) ||
             code.test(BATTERY_STATUS_OVER_TEMP_BIT_POS) ||
             code.test(BATTERY_STATUS_FULLY_CHARGED_BIT_POS));
}