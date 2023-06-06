#include "battery_mock.h"

BatteryMock::BatteryMock()
{}

int BatteryMock::TriggerGeneralSampling()
{
#ifdef CONFIG_UNIT_TEST_MOCKS
    gen_data_sampled.temp = gen_data_mock.temp;
    gen_data_sampled.volt = gen_data_mock.volt;
    gen_data_sampled.current = gen_data_mock.current;
    gen_data_sampled.remaining_capacity = gen_data_mock.remaining_capacity;
    gen_data_sampled.relative_charge_state = gen_data_mock.relative_charge_state;
    gen_data_sampled.cycle_count = gen_data_mock.cycle_count;
#endif  //_CONFIG_UNIT_TEST_MOCKS_

    return 0;
}

int BatteryMock::TriggerChargingSampling()
{
#ifdef CONFIG_UNIT_TEST_MOCKS
    chg_data_sampled.status = chg_data_mock.status;
    chg_data_sampled.des_chg_current = chg_data_mock.des_chg_current;
    chg_data_sampled.des_chg_volt = chg_data_mock.des_chg_volt;
#endif  //_CONFIG_UNIT_TEST_MOCKS_

    return 0;
}

int BatteryMock::GetGeneralData(BatteryGeneralData &bat_gen_data)
{
    bat_gen_data.temp = gen_data_sampled.temp;
    bat_gen_data.volt = gen_data_sampled.volt;
    bat_gen_data.current = gen_data_sampled.current;
    bat_gen_data.remaining_capacity = gen_data_sampled.remaining_capacity;
    bat_gen_data.relative_charge_state = gen_data_sampled.relative_charge_state;
    bat_gen_data.cycle_count = gen_data_sampled.cycle_count;
    return 0;
}
int BatteryMock::GetChargingData(BatteryChargingData &bat_chg_data)
{
    bat_chg_data.des_chg_current = chg_data_sampled.des_chg_current;
    bat_chg_data.des_chg_volt = chg_data_sampled.des_chg_volt;
    bat_chg_data.status = chg_data_sampled.status;
    return 0;
}

int BatteryMock::GetTemperature(float &temp)
{
    temp = gen_data_sampled.temp;
    return 0;
}

int BatteryMock::GetVoltage(float &volt)
{
    volt = gen_data_sampled.volt;
    return 0;
}

int BatteryMock::GetCurrent(float &current)
{
    current = gen_data_sampled.current;
    return 0;
}

int BatteryMock::GetRemCapacity(int32_t &rem_cap)
{
    rem_cap = gen_data_sampled.remaining_capacity;
    return 0;
}

int BatteryMock::GetRelativeStateOfCharge(int32_t &relative_charge_state)
{
    relative_charge_state = gen_data_sampled.relative_charge_state;
    return 0;
}

int BatteryMock::GetCycleCount(int32_t &cycle_count)
{
    cycle_count = gen_data_sampled.cycle_count;
    return 0;
}

int BatteryMock::GetChargingCurrent(int32_t &charging_current)
{
    charging_current = chg_data_sampled.des_chg_current;
    return 0;
}

int BatteryMock::GetChargingVoltage(int32_t &charging_volt)
{
    charging_volt = chg_data_sampled.des_chg_volt;
    return 0;
}

int BatteryMock::GetStatus(int32_t &status)
{
    status = chg_data_sampled.status;
    return 0;
}

bool BatteryMock::CanBeCharged(int32_t status_code)
{
    return false;
}

#ifdef CONFIG_UNIT_TEST_MOCKS
void BatteryMock::SetTestTemp(float test_val)
{
    gen_data_mock.temp = test_val;
}

void BatteryMock::SetTestVolt(float test_val)
{
    gen_data_mock.volt = test_val;
}

void BatteryMock::SetTestCurrent(float test_val)
{
    gen_data_mock.current = test_val;
}

void BatteryMock::SetTestRemCap(int32_t test_val)
{
    gen_data_mock.remaining_capacity = test_val;
}

void BatteryMock::SetTestRelChargeState(int32_t test_val)
{
    gen_data_mock.relative_charge_state = test_val;
}

void BatteryMock::SetTestCycleCount(int32_t test_val)
{
    gen_data_mock.cycle_count = test_val;
}

void BatteryMock::SetTestStatus(int32_t test_val)
{
    chg_data_mock.status = test_val;
}

void BatteryMock::SetTestChargingCurrent(int32_t test_val)
{
    chg_data_mock.des_chg_current = test_val;
}

void BatteryMock::SetTestChargingVoltage(int32_t test_val)
{
    chg_data_mock.des_chg_volt = test_val;
}
#endif