#pragma once

#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include "battery.h"

#define BATTERY_STATUS_OVER_CHARGED_BIT_POS 15
#define BATTERY_STATUS_TERMINATE_CHARGE_BIT_POS 14
#define BATTERY_STATUS_OVER_TEMP_BIT_POS 12
#define BATTERY_STATUS_TERMINATE_DISCHARGE_BIT_POS 11
#define BATTERY_STATUS_INITIALIZED_BIT_POS 7
#define BATTERY_STATUS_DISCHARGING_BIT_POS 6
#define BATTERY_STATUS_FULLY_CHARGED_BIT_POS 5
#define BATTERY_STATUS_FULLY_DISCHARGED_BIT_POS 4

#define KELVIN_CELSIUS_DIFF 273.15
#define NUM_GENERAL_DATA 5
#define NUM_CHARGING_DATA 4

class BatteryNh2054qe34 : public Battery {
public:
    BatteryNh2054qe34();
    virtual ~BatteryNh2054qe34() = default;
    int Init() override;

    int TriggerGeneralSampling() override;
    int TriggerChargingSampling() override;
    int GetGeneralData(BatteryGeneralData &data) override;
    int GetChargingData(BatteryChargingData &data) override;

    ///  \note (resolution of 0.1).
    int GetTemperature(float &temp) override;

    int GetVoltage(float &volt) override;
    int GetCurrent(float &current) override;

    /// \note Return Milliampere hours (mAh) if is battery configured with CAPACITY_MODE 0
    /// (default).
    ///              Milliwatt hours (mWh) if battery is configured with CAPACITY_MODE 1.
    int GetRemCapacity(int32_t &rem_cap) override;

    int GetRelativeStateOfCharge(int32_t &relative_charge_state) override;
    int GetCycleCount(int32_t &cycle_count) override;
    int GetChargingCurrent(int32_t &charging_current) override;
    int GetChargingVoltage(int32_t &charging_volt) override;
    int GetStatus(int32_t &status) override;
    bool CanBeCharged(int32_t status_code) override;

private:
    const struct device *battery_dev_ = DEVICE_DT_GET(DT_NODELABEL(smart_battery));
};