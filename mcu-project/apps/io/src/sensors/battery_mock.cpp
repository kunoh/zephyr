#include "battery_mock.h"

BatteryMock::BatteryMock()
{}

int BatteryMock::TriggerSampling()
{
#ifdef CONFIG_UNIT_TEST_MOCKS
    temp_sampled = temp_mock;
    volt_sampled = volt_mock;
    current_sampled = current_mock;
    rem_cap_sampled = rem_cap_mock;
    status_sampled = status_mock;
    rel_charge_state_sampled = rel_charge_state_mock;
    cycle_count_sampled = cycle_count_mock;
#endif  //_CONFIG_UNIT_TEST_MOCKS_

    return 0;
}

int BatteryMock::GetTemperature(float *temp)
{
    *temp = temp_sampled;
    return 0;
}

int BatteryMock::GetVoltage(float *volt)
{
    *volt = volt_sampled;
    return 0;
}

int BatteryMock::GetCurrent(float *current)
{
    *current = current_sampled;
    return 0;
}

int BatteryMock::GetRemCapacity(uint32_t *rem_cap)
{
    *rem_cap = rem_cap_sampled;
    return 0;
}

int BatteryMock::GetStatus(uint16_t *status)
{
    *status = status_sampled;
    return 0;
}

int BatteryMock::GetRelativeStateOfCharge(uint8_t *relative_charge_state)
{
    *relative_charge_state = rel_charge_state_sampled;
    return 0;
}

int BatteryMock::GetCycleCount(uint16_t *cycle_count)
{
    *cycle_count = cycle_count_sampled;
    return 0;
}

#ifdef CONFIG_UNIT_TEST_MOCKS
void BatteryMock::SetTestTemp(float test_val)
{
    temp_mock = test_val;
}

void BatteryMock::SetTestVolt(float test_val)
{
    volt_mock = test_val;
}

void BatteryMock::SetTestCurrent(float test_val)
{
    current_mock = test_val;
}

void BatteryMock::SetTestRemCap(uint32_t test_val)
{
    rem_cap_mock = test_val;
}

void BatteryMock::SetTestStatus(uint16_t test_val)
{
    status_mock = test_val;
}

void BatteryMock::SetTestRelChargeState(uint8_t test_val)
{
    rel_charge_state_mock = test_val;
}

void BatteryMock::SetTestCycleCount(uint16_t test_val)
{
    cycle_count_mock = test_val;
}
#endif