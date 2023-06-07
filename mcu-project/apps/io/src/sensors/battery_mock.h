#pragma once

#include "battery.h"

class BatteryMock : public Battery {
public:
    BatteryMock();
    virtual ~BatteryMock() = default;
    int Init() override;

    int TriggerGeneralSampling() override;
    int TriggerChargingSampling() override;
    int GetGeneralData(BatteryGeneralData &bat_gen_data) override;
    int GetChargingData(BatteryChargingData &bat_chg_data) override;
    int GetTemperature(float &temp) override;
    int GetVoltage(float &volt) override;
    int GetCurrent(float &current) override;
    int GetRemCapacity(int32_t &rem_cap) override;
    int GetRelativeStateOfCharge(int32_t &relative_charge_state) override;
    int GetCycleCount(int32_t &cycle_count) override;
    int GetChargingCurrent(int32_t &charging_current) override;
    int GetChargingVoltage(int32_t &charging_volt) override;
    int GetStatus(int32_t &status) override;
    bool CanBeCharged(int32_t status_code);

#ifdef CONFIG_UNIT_TEST_MOCKS
    // Set "internal" battery properties for test purposes.
    void SetTestTemp(float test_val);
    void SetTestVolt(float test_val);
    void SetTestCurrent(float test_val);
    void SetTestRemCap(int32_t test_val);
    void SetTestRelChargeState(int32_t test_val);
    void SetTestCycleCount(int32_t test_val);
    void SetTestChargingCurrent(int32_t test_val);
    void SetTestChargingVoltage(int32_t test_val);
    void SetTestStatus(int32_t test_val);
#endif  //_CONFIG_UNIT_TEST_MOCKS_

private:  // Mocked "internal" and "sampled" battery properties.
    BatteryGeneralData gen_data_mock, gen_data_sampled;
    BatteryChargingData chg_data_mock, chg_data_sampled;
};