#pragma once

#include "battery.h"
#include "logger.h"

#define KELVIN_CELSIUS_DIFF 273.15

class BatteryNh2054qe34 : public Battery {
public:
    BatteryNh2054qe34(Logger &logger);
    virtual ~BatteryNh2054qe34() = default;

    int TriggerSampling() override;

    ///  \note (resolution of 0.1).
    int GetTemperature(float *temp) override;

    int GetVoltage(float *volt) override;
    int GetCurrent(float *current) override;

    /// \note Return Milliampere hours (mAh) if is battery configured with CAPACITY_MODE 0
    /// (default).
    ///              Milliwatt hours (mWh) if battery is configured with CAPACITY_MODE 1.
    int GetRemCapacity(uint32_t *rem_cap) override;

    int GetStatus(uint16_t *status) override;
    int GetRelativeStateOfCharge(uint8_t *relative_charge_state) override;
    int GetCycleCount(uint16_t *cycle_count) override;

private:
    Logger &logger_;
};