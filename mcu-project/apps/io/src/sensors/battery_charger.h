#pragma once

#include <stdint.h>

#define ERRNO_CHARGER_CURRENT_EIO 69
#define ERRNO_CHARGER_CURRENT_ERANGE 98
#define ERRNO_CHARGER_VOLTAGE_EIO 133
#define ERRNO_CHARGER_VOLTAGE_ERANGE 162

class BatteryCharger {
public:
    virtual ~BatteryCharger() = default;
    virtual int Init() = 0;

    virtual int SetChargingCurrent(int32_t chg_current) = 0;
    virtual int SetChargingVoltage(int32_t chg_volt) = 0;

    // clang-format off
    ///
    /// @brief Attempts to set both the max charging voltage and charging current of the battery
    /// charge controller in one bulk operation.
    ///
    /// @note Voltage is always configured before current. If voltage configuration fails, current
    /// configuration is not attempted.
    ///
    /// @param[in] chg_current The charging current to set.
    /// @param[in] chg_volt The max charging voltage to set.
    ///
    /// @return 0 on success.
    ///         ERRNO_CHARGER_VOLTAGE_EIO on fail to configure charger with max charging voltage.
    ///         ERRNO_CHARGER_VOLTAGE_ERANGE if requested voltage setting is outside charger operating range. 
    ///         ERRNO_CHARGER_CURRENT_EIO on fail to configure charger with charging current. 
    ///         ERRNO_CHARGER_CURRENT_ERANGE if requested current setting is outside charger operating range. Non-zero errno otherwise.
    ///
    virtual int SetChargingConfig(int32_t chg_current, int32_t chg_volt) = 0;
// clang format-on

    virtual int GetChargerStatus(int32_t &chgr_status) = 0;
};