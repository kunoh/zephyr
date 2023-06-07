#pragma once

#include <zephyr/device.h>

#include "battery_charger.h"
#include "logger.h"
#include "util.h"

#define BATTERY_STATUS_OVER_CHARGED_BIT_POS 15
#define BATTERY_STATUS_TERMINATE_CHARGE_BIT_POS 14
#define BATTERY_STATUS_OVER_TEMP_BIT_POS 12
#define BATTERY_STATUS_TERMINATE_DISCHARGE_BIT_POS 11
#define BATTERY_STATUS_INITIALIZED_BIT_POS 7
#define BATTERY_STATUS_DISCHARGING_BIT_POS 6
#define BATTERY_STATUS_FULLY_CHARGED_BIT_POS 5
#define BATTERY_STATUS_FULLY_DISCHARGED_BIT_POS 4

class BatteryChargerBq25713 : public BatteryCharger {
public:
    BatteryChargerBq25713(Logger &logger);
    virtual ~BatteryChargerBq25713() = default;
    int Init() override;

    int SetChargingCurrent(int32_t chg_current) override;
    int SetChargingVoltage(int32_t chg_volt) override;
    int SetChargingConfig(int32_t chg_current, int32_t chg_volt) override;
    int GetChargerStatus(int32_t &chgr_status) override;

private:
    Logger &logger_;
    const struct device *charger_dev = DEVICE_DT_GET(DT_NODELABEL(smart_battery_charger));
};