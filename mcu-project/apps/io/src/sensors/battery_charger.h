#pragma once

#include <stdint.h>

class BatteryCharger {
public:
    virtual ~BatteryCharger() = default;

    virtual int SetChargingCurrent(int32_t chg_current) = 0;

    virtual int SetChargingVoltage(int32_t chg_volt) = 0;

    ///
    /// @brief Sets both the max charging voltage and charging current of the battery charge
    /// controller in one bulk operation.
    ///
    /// @param[in] chg_current The charging current to set.
    /// @param[in] chg_volt The max charging voltage to set.
    /// @param[out] current_set_success Boolean status whether setting the current succeeded.
    /// @param[out] current_set_volt Boolean status whether setting the max voltage succeeded.
    ///
    /// @return 0 on success, non-zero otherwise. Additionally, if current_set_success or
    /// volt_set_success are false,
    ///         the corresponding bits will also be set in the return code.
    ///
    virtual int SetChargingConfig(int32_t chg_current, int32_t chg_volt, bool &current_set_success,
                                  bool &volt_set_success) = 0;

    virtual int GetChargerStatus(int32_t &chgr_status) = 0;
};