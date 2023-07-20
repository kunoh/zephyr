#include "battery_nh2054qe34.h"

#include <nh2054qe34_driver.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bat_nh2054qe34, CONFIG_BATTERY_LOG_LEVEL);

BatteryNh2054qe34::BatteryNh2054qe34()
{}

int BatteryNh2054qe34::Init()
{
    int ret;
    ret = sensor_attr_set(battery_dev_, (sensor_channel)SENSOR_CHAN_INIT_CONFIG,
                          SENSOR_ATTR_CONFIGURATION, NULL);

    if (ret != 0 || !device_is_ready(battery_dev_)) {
        LOG_ERR("NH2054QE34 battery not found. Aborting...");
        return 1;
    }

    LOG_INF("Battery NH2054QE34 initialized.");
    return 0;
}

int BatteryNh2054qe34::TriggerGeneralSampling()
{
    rcode_sampling_gen_ = std::bitset<CHAR_BIT * sizeof(int)>(
        sensor_sample_fetch_chan(battery_dev_, (sensor_channel)SENSOR_CHAN_BATTERY_GENERAL));

    return (int)rcode_sampling_gen_.to_ulong();
}

int BatteryNh2054qe34::TriggerChargingSampling()
{
    rcode_sampling_chg_ = std::bitset<CHAR_BIT * sizeof(int)>(
        sensor_sample_fetch_chan(battery_dev_, (sensor_channel)SENSOR_CHAN_BATTERY_CHARGING));
    return (int)rcode_sampling_chg_.to_ulong();
}

int BatteryNh2054qe34::GetGeneralData(BatteryGeneralData &data)
{
    int ret = 0;

    ret = GetTemperature(data.temp);
    ret |= GetCurrent(data.current);
    ret |= GetVoltage(data.volt);
    ret |= GetRemCapacity(data.remaining_capacity);
    ret |= GetCycleCount(data.cycle_count);

    if (ret != 0) {
        LOG_WRN("Failed to get one or more general battery properties.");
    }

    return ret;
}

int BatteryNh2054qe34::GetChargingData(BatteryChargingData &data)
{
    int ret = 0;

    ret = GetChargingCurrent(data.des_chg_current);
    ret |= GetChargingVoltage(data.des_chg_volt);
    ret |= GetStatus(data.status);
    ret |= GetRelativeStateOfCharge(data.relative_charge_state);

    if (ret != 0) {
        LOG_WRN("Failed to get one or more battery charging properties.");
    }

    return ret;
}

int BatteryNh2054qe34::GetTemperature(float &temp)
{
    int ret = 0;
    struct sensor_value battery_temp;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_TEMP, &battery_temp);

    if (ret == 0 && !rcode_sampling_gen_.test(ERR_TEMP_BIT_POS)) {
        temp = (sensor_value_to_double(&battery_temp) / 10) - KELVIN_CELSIUS_DIFF;
    } else {
        temp = DEFAULT_INVALID_BAT_FLOAT;
    }

    return ret;
}

int BatteryNh2054qe34::GetCurrent(float &current)
{
    int ret = 0;
    struct sensor_value battery_current;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_CURRENT, &battery_current);

    if (ret == 0 && !rcode_sampling_gen_.test(ERR_CURRENT_BIT_POS)) {
        current = sensor_value_to_double(&battery_current);
    } else {
        current = DEFAULT_INVALID_BAT_FLOAT;
    }

    return ret;
}

int BatteryNh2054qe34::GetVoltage(float &volt)
{
    int ret = 0;
    struct sensor_value battery_volt;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_VOLTAGE, &battery_volt);

    if (ret == 0 && !rcode_sampling_gen_.test(ERR_VOLTAGE_BIT_POS)) {
        volt = sensor_value_to_double(&battery_volt);
    } else {
        volt = DEFAULT_INVALID_BAT_FLOAT;
    }

    return ret;
}

int BatteryNh2054qe34::GetRemCapacity(int32_t &rem_cap)
{
    int ret = 0;
    struct sensor_value battery_rem_capacity;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_REMAINING_CHARGE_CAPACITY,
                             &battery_rem_capacity);

    if (ret == 0 && !rcode_sampling_gen_.test(ERR_REM_CAP_BIT_POS)) {
        rem_cap = battery_rem_capacity.val1;
    } else {
        rem_cap = DEFAULT_INVALID_BAT_INT;
    }
    return ret;
}

int BatteryNh2054qe34::GetCycleCount(int32_t &cycle_count)
{
    int ret = 0;
    sensor_value battery_cycle_count;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_CYCLE_COUNT, &battery_cycle_count);

    if (ret == 0 && !rcode_sampling_gen_.test(ERR_CYCLE_COUNT_BIT_POS)) {
        cycle_count = battery_cycle_count.val1;
    } else {
        cycle_count = DEFAULT_INVALID_BAT_INT;
    }
    return ret;
}

int BatteryNh2054qe34::GetChargingCurrent(int32_t &charging_current)
{
    int ret = 0;
    struct sensor_value battery_charging_current;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT,
                             &battery_charging_current);

    if (ret == 0 && !rcode_sampling_chg_.test(ERR_CHARGING_CURRENT_BIT_POS)) {
        charging_current = battery_charging_current.val1;
    } else {
        charging_current = DEFAULT_INVALID_BAT_INT;
    }

    return ret;
}

int BatteryNh2054qe34::GetChargingVoltage(int32_t &charging_volt)
{
    int ret = 0;
    struct sensor_value battery_charging_volt;

    ret =
        sensor_channel_get(battery_dev_, (sensor_channel)SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE,
                           &battery_charging_volt);

    if (ret == 0 && !rcode_sampling_chg_.test(ERR_CHARGING_VOLTAGE_BIT_POS)) {
        charging_volt = battery_charging_volt.val1;
    } else {
        charging_volt = DEFAULT_INVALID_BAT_INT;
    }

    return ret;
}

int BatteryNh2054qe34::GetStatus(int32_t &status)
{
    int ret = 0;
    struct sensor_value battery_status;

    ret = sensor_channel_get(battery_dev_, (sensor_channel)SENSOR_CHAN_STATUS, &battery_status);

    if (ret == 0 && !rcode_sampling_chg_.test(ERR_STATUS_BIT_POS)) {
        status = battery_status.val1;
    } else {
        status = DEFAULT_INVALID_BAT_INT;
    }

    return ret;
}

int BatteryNh2054qe34::GetRelativeStateOfCharge(int32_t &relative_charge_state)
{
    int ret = 0;
    sensor_value battery_state_of_charge;

    ret = sensor_channel_get(battery_dev_, SENSOR_CHAN_GAUGE_STATE_OF_CHARGE,
                             &battery_state_of_charge);

    if (ret == 0 && !rcode_sampling_chg_.test(ERR_REL_CHARGE_STATE_BIT_POS)) {
        relative_charge_state = battery_state_of_charge.val1;
    } else {
        relative_charge_state = DEFAULT_INVALID_BAT_INT;
    }

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