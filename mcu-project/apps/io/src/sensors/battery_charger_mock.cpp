#include "battery_charger_mock.h"

BatteryChargerMock::BatteryChargerMock()
{}

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