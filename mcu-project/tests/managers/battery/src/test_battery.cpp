#include <zephyr/ztest.h> // https://github.com/zephyrproject-rtos/zephyr/issues/55450
#include <zephyr/kernel.h>
#include <stdio.h>

#include "battery_manager.h"
#include "battery_mock.h"
#include "battery_charger_mock.h"
#include "util.h"
#include "logger_zephyr.h"

static void test_cb(BatteryGeneralData data)
{
    return;
}

ZTEST_SUITE(battery_manager_suite, NULL, NULL, NULL, NULL, NULL);

// Test that BatteryManager.TriggerAndGetSample(...) triggers a battery sampling 
// and then fetches the individual battery properties using the Battery interface.
ZTEST(battery_manager_suite, test_get_last_general_data)
{
    // Setup some dummy variables and assign them to the mocked battery
    BatteryGeneralData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.relative_charge_state = 90;
    test_data_exp.cycle_count = 120;

    LoggerZephyr logger("");
    std::unique_ptr<BatteryMock> test_bat_mock = std::make_unique<BatteryMock>();
    std::unique_ptr<BatteryCharger> test_chg = std::make_unique<BatteryChargerMock>();
    test_bat_mock->SetTestTemp(test_data_exp.temp);
    test_bat_mock->SetTestVolt(test_data_exp.volt);
    test_bat_mock->SetTestCurrent(test_data_exp.current);
    test_bat_mock->SetTestRemCap(test_data_exp.remaining_capacity);
    test_bat_mock->SetTestRelChargeState(test_data_exp.relative_charge_state);
    test_bat_mock->SetTestCycleCount(test_data_exp.cycle_count);

    std::unique_ptr<Battery> test_battery(std::move(test_bat_mock));
    BatteryManager battery_mngr(std::make_shared<LoggerZephyr>(logger), std::move(test_battery), std::move(test_chg));
    BatteryGeneralData test_data_res;
    zassert_not_equal(battery_mngr.GetLastGeneralData(test_data_res), 0);
    zassert_equal(test_data_res.temp, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.volt, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.current, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.remaining_capacity, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.relative_charge_state, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.cycle_count, DEFAULT_INVALID_BAT_INT);

    battery_mngr.StartSampling(GENERAL, 20, 60000);
    k_msleep(30);
    
    zassert_ok(battery_mngr.GetLastGeneralData(test_data_res));
    battery_mngr.StopSampling(GENERAL);
    zassert_equal(test_data_res.temp, test_data_exp.temp);
    zassert_equal(test_data_res.volt, test_data_exp.volt);
    zassert_equal(test_data_res.current, test_data_exp.current);
    zassert_equal(test_data_res.remaining_capacity, test_data_exp.remaining_capacity);
    zassert_equal(test_data_res.relative_charge_state, test_data_exp.relative_charge_state);
    zassert_equal(test_data_res.cycle_count, test_data_exp.cycle_count);
};

ZTEST(battery_manager_suite, test_adding_clearing_gen_subscribers)
{
    LoggerZephyr logger("");
    std::unique_ptr<Battery> test_battery = std::make_unique<BatteryMock>();
    std::unique_ptr<BatteryCharger> test_chg = std::make_unique<BatteryChargerMock>();
    BatteryManager battery_mngr(std::make_shared<LoggerZephyr>(logger), std::move(test_battery), std::move(test_chg));
    zassert_false(battery_mngr.CpuIsSubscribed());
    zassert_equal(battery_mngr.GetSubscriberCount(GENERAL), 0);

    battery_mngr.AddSubscriberGeneral(test_cb);
    battery_mngr.SetCpuSubscribed(true);
    zassert_true(battery_mngr.CpuIsSubscribed());
    zassert_equal(battery_mngr.GetSubscriberCount(GENERAL), 1);

    battery_mngr.ClearSubscribers(GENERAL);
    zassert_false(battery_mngr.CpuIsSubscribed());
    zassert_equal(battery_mngr.GetSubscriberCount(GENERAL), 0);
}