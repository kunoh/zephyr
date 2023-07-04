#include "battery_charger_mock.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bat_chgr_mock, CONFIG_BATTERY_CHARGER_LOG_LEVEL);

BatteryChargerMock::BatteryChargerMock()
{}

int BatteryChargerMock::Init()
{
    LOG_INF("Battery charger mock initialized.");
    return 0;
}

int BatteryChargerMock::SetChargingCurrent(int32_t chg_current)
{
    return 0;
}

int BatteryChargerMock::SetChargingVoltage(int32_t chg_volt)
{
    return 0;
}

int BatteryChargerMock::SetChargingConfig(int32_t chg_current, int32_t chg_volt)
{
    return 0;
}

int BatteryChargerMock::GetChargerStatus(int32_t &chgr_status)
{
    return 0;
}