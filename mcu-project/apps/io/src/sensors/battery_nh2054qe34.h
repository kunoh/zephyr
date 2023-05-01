#pragma once

#include "battery.h"
#include "logger.h"

#define KELVIN_CELSIUS_DIFF 273.15
#define NUM_GENERAL_DATA 6
#define NUM_CHARGING_DATA 3

class BatteryNh2054qe34 : public Battery {
public:
    BatteryNh2054qe34(Logger &logger);
    virtual ~BatteryNh2054qe34() = default;

    int TriggerGeneralSampling() override;
    int TriggerChargingSampling() override;
    int GetGeneralData(BatteryGeneralData *data) override;
    int GetChargingData(BatteryChargingData *data) override;

    ///  \note (resolution of 0.1).
    int GetTemperature(float *temp) override;

    int GetVoltage(float *volt) override;
    int GetCurrent(float *current) override;

    /// \note Return Milliampere hours (mAh) if is battery configured with CAPACITY_MODE 0
    /// (default).
    ///              Milliwatt hours (mWh) if battery is configured with CAPACITY_MODE 1.
    int GetRemCapacity(int32_t *rem_cap) override;

    int GetRelativeStateOfCharge(int32_t *relative_charge_state) override;
    int GetCycleCount(int32_t *cycle_count) override;
    int GetChargingCurrent(int32_t *charging_current) override;
    int GetChargingVoltage(int32_t *charging_volt) override;
    int GetStatus(int32_t *status) override;

private:
    Logger &logger_;
};