#pragma once

#include <zephyr/drivers/sensor.h>

class Battery {
public:
    virtual ~Battery() = default;

    ///
    /// @brief Trigger a sampling of current battery properties.
    ///
    /// @retval 0 on success. Non-zero otherwise.
    ///
    virtual int TriggerSampling() = 0;

    ///
    /// @brief Get last fetched battery temperature sample.
    ///
    /// Unit: Degrees Celsius.
    ///
    /// @param[out] temp Pointer to write temperature sample to.
    ///
    /// @retval 0 on success. Non-zero otherwise.
    ///
    virtual int GetTemperature(float *temp) = 0;

    ///
    /// @brief Get last fetched battery voltage sample.
    ///
    /// Unit: Volts (V).
    ///
    /// @param[out] volt Pointer to write voltage sample to.
    ///
    /// @retval 0 on success. Non-zero otherwise.
    ///
    virtual int GetVoltage(float *volt) = 0;

    ///
    /// @brief Get last fetched battery current sample.
    ///
    /// Returns the current being supplied (or accepted) through the battery's terminals.
    ///   Unit: Amperes (A). Positive for charge, negative for discharge.
    ///
    /// @param[out] volt Pointer to write current sample to.
    ///
    /// @retval 0 on success. Non-zero otherwise.
    ///
    virtual int GetCurrent(float *current) = 0;

    ///
    /// @brief Get last fetched remaining battery capacity sample.
    ///
    /// Returns the battery's estimated remaining capacity in Milliampere hours (mAh).
    ///
    /// @param[out] rem_cap Pointer to write remaining capacity sample to.
    ///
    /// @return 0 on success. Non-zero otherwise.
    ///
    virtual int GetRemCapacity(uint32_t *rem_cap) = 0;

    ///
    /// @brief Get last fetched battery status sample.
    ///
    /// See Smart Battery Data Specification rev. 1.0 for details.
    ///
    /// @param[out] status Pointer to write status sample to.
    ///
    /// @return 0 on success. Non-zero otherwise.
    ///
    virtual int GetStatus(uint16_t *status) = 0;

    ///
    /// @brief Get last fetched sample of information on the battery relative state of charge.
    ///
    /// Get an estimation of the remaining battery capacity expressed as a percentage (0 to 100%) of
    /// the full charge capacity.
    ///
    /// @param[out] relative_charge_state Pointer to struct to write charge state to.
    ///
    /// @return 0 on success. Non-zero otherwise.
    ///
    virtual int GetRelativeStateOfCharge(uint8_t *relative_charge_state) = 0;

    ///
    /// @brief Get last fetched sample of the battery cycle count.
    ///
    /// The cycle count is a number between 0 and 65534.
    /// A cycle count of 65535 indicates the battery has been cycled more than 65534 times.
    ///
    /// @param[out] cycle_count Pointer to write cycle count to.
    ///
    /// @return 0 on success. Non-zero otherwise.
    ///
    virtual int GetCycleCount(uint16_t *cycle_count) = 0;
};