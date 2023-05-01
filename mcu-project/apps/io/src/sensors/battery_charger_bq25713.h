#pragma once

#include <zephyr/device.h>

#include "battery_charger.h"
#include "logger.h"
#include "util.h"

class BatteryChargerBq25713 : public BatteryCharger {
public:
    BatteryChargerBq25713(Logger &logger);
    virtual ~BatteryChargerBq25713() = default;

    int SetChargingCurrent(int32_t chg_current) override;

    int SetChargingVoltage(int32_t chg_volt) override;

    int SetChargingConfig(int32_t chg_current, int32_t chg_volt, bool &current_set_success,
                          bool &volt_set_success) override;

    int GetChargerStatus(int32_t &chgr_status) override;

private:
    Logger &logger_;
    const struct device *charger_dev = DEVICE_DT_GET(DT_NODELABEL(smart_battery_charger));
};