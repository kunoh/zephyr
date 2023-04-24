#pragma once

#include "battery.h"

class BatteryMock : public Battery {
public:
    BatteryMock();
    virtual ~BatteryMock() = default;

    int TriggerSampling() override;
    int GetTemperature(float *temp) override;
    int GetVoltage(float *volt) override;
    int GetCurrent(float *current) override;
    int GetRemCapacity(uint32_t *rem_cap) override;
    int GetStatus(uint16_t *status) override;
    int GetRelativeStateOfCharge(uint8_t *relative_charge_state) override;
    int GetCycleCount(uint16_t *cycle_count) override;

#ifdef CONFIG_UNIT_TEST_MOCKS
    // Set "internal" battery properties for test purposes.
    void SetTestTemp(float test_val);
    void SetTestVolt(float test_val);
    void SetTestCurrent(float test_val);
    void SetTestRemCap(uint32_t test_val);
    void SetTestStatus(uint16_t test_val);
    void SetTestRelChargeState(uint8_t test_val);
    void SetTestCycleCount(uint16_t test_val);
#endif  //_CONFIG_UNIT_TEST_MOCKS_

private:
    // Mocked "internal" and "sampled" battery properties.
    float temp_mock, temp_sampled = static_cast<float>(0);
    float volt_mock, volt_sampled = static_cast<float>(0);
    float current_mock, current_sampled = static_cast<float>(0);
    uint32_t rem_cap_mock, rem_cap_sampled = 0;
    uint16_t status_mock, status_sampled = 0x00;
    uint8_t rel_charge_state_mock, rel_charge_state_sampled = 0;
    uint16_t cycle_count_mock, cycle_count_sampled = 0;
};