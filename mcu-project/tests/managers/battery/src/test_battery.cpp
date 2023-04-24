#include <zephyr/ztest.h> // https://github.com/zephyrproject-rtos/zephyr/issues/55450
#include <zephyr/kernel.h>
#include <stdio.h>

#include "battery_manager.h"
#include "battery_mock.h"
#include "util.h"
#include "logger_zephyr.h"

static void test_cb(BatteryData data)
{
    return;
}

ZTEST_SUITE(battery_manager_suite, NULL, NULL, NULL, NULL, NULL);

// Test that BatteryManager.TriggerAndGetSample(...) triggers a battery sampling 
// and then fetches the individual battery properties using the Battery interface.
ZTEST(battery_manager_suite, test_trigger_and_get_a_sample)
{
    // Setup some dummy variables and assign them to the mocked battery
    BatteryData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.status = 0xff;
    test_data_exp.relative_charge_state = 90;
    test_data_exp.cycle_count = 120;

    LoggerZephyr logger("");
    std::unique_ptr<BatteryMock> test_mock = std::make_unique<BatteryMock>();
    test_mock->SetTestTemp(test_data_exp.temp);
    test_mock->SetTestVolt(test_data_exp.volt);
    test_mock->SetTestCurrent(test_data_exp.current);
    test_mock->SetTestRemCap(test_data_exp.remaining_capacity);
    test_mock->SetTestStatus(test_data_exp.status);
    test_mock->SetTestRelChargeState(test_data_exp.relative_charge_state);
    test_mock->SetTestCycleCount(test_data_exp.cycle_count);

    std::unique_ptr<Battery> test_battery(std::move(test_mock));
    BatteryManager battery_mngr(std::make_shared<LoggerZephyr>(logger), std::move(test_battery));

    // Test
    BatteryData test_data_res;
    zassert_ok(battery_mngr.TriggerAndGetSample(&test_data_res));
    zassert_equal(test_data_res.temp, test_data_exp.temp);
    zassert_equal(test_data_res.volt, test_data_exp.volt);
    zassert_equal(test_data_res.current, test_data_exp.current);
    zassert_equal(test_data_res.remaining_capacity, test_data_exp.remaining_capacity);
    zassert_equal(test_data_res.status, test_data_exp.status);
    zassert_equal(test_data_res.relative_charge_state, test_data_exp.relative_charge_state);
    zassert_equal(test_data_res.cycle_count, test_data_exp.cycle_count);
};

ZTEST(battery_manager_suite, test_adding_clearing_subscribers)
{
    LoggerZephyr logger("");
    std::unique_ptr<Battery> test_battery = std::make_unique<BatteryMock>();
    BatteryManager battery_mngr(std::make_shared<LoggerZephyr>(logger), std::move(test_battery));
    zassert_false(battery_mngr.GetCpuSubscribed());
    zassert_equal(battery_mngr.GetSubscriberCount(), 0);

    battery_mngr.AddSubscriber(test_cb);
    battery_mngr.SetCpuSubscribed(true);
    zassert_true(battery_mngr.GetCpuSubscribed());
    zassert_equal(battery_mngr.GetSubscriberCount(), 1);

    battery_mngr.ClearSubscribers();
    zassert_false(battery_mngr.GetCpuSubscribed());
    zassert_equal(battery_mngr.GetSubscriberCount(), 0);
}