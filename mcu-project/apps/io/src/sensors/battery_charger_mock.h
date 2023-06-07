#pragma once

#include "battery_charger.h"

class BatteryChargerMock : public BatteryCharger {
public:
    BatteryChargerMock();
    virtual ~BatteryChargerMock() = default;
    int Init() override;

    int SetChargingCurrent(int32_t chg_current) override;

    int SetChargingVoltage(int32_t chg_volt) override;

    int SetChargingConfig(int32_t chg_current, int32_t chg_volt) override;

    int GetChargerStatus(int32_t &chgr_status) override;
};