#include <zephyr/ztest.h> // https://github.com/zephyrproject-rtos/zephyr/issues/55450
#include <zephyr/kernel.h>
#include <stdio.h>

#include "battery_manager.h"
#include "battery_mock.h"
#include "battery_charger_mock.h"
#include "util.h"

static int test_cb_gen(BatteryGeneralData data)
{
    static int tester = 0;
    tester += data.cycle_count;
    return tester;
}
static int test_cb_chg(BatteryChargingData data)
{
    static int tester = 0;
    tester += data.des_chg_current;
    return tester;
}

ZTEST_SUITE(battery_manager_suite, NULL, NULL, NULL, NULL, NULL);


ZTEST(battery_manager_suite, test_set_get_installation_mode)
{
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    BatteryManager battery_mngr(test_bat_mock, test_chg);

    // Uninitialized manager returns errno
    zassert_equal(battery_mngr.GetInstallationMode(), "MOBILE");
    for(int i = 0; i < NUM_INSTALLATION_MODES; i++)
    {
        zassert_equal(battery_mngr.SetInstallationMode(installation_modes[i]), EINVAL);
        zassert_equal(battery_mngr.GetInstallationMode(), "MOBILE");
    }

    // Initialized manager sets mode as expected
    battery_mngr.Init();
    for(int i = 0; i < NUM_INSTALLATION_MODES; i++)
    {
        zassert_ok(battery_mngr.SetInstallationMode(installation_modes[i]));
        zassert_equal(battery_mngr.GetInstallationMode(), installation_modes[i]);
    }

    // Attempt setting unknown mode
    std::string val = battery_mngr.GetInstallationMode();
    zassert_equal(battery_mngr.SetInstallationMode("UNKNOWN_MODE"), EINVAL);
    zassert_equal(battery_mngr.GetInstallationMode(), val);

    battery_mngr.StopSampling("GENERAL");
    battery_mngr.StopSampling("CHARGING");
}

ZTEST(battery_manager_suite, test_initialized_set_get_chg_limit)
{
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    BatteryManager battery_mngr(test_bat_mock, test_chg);

    int32_t limit_in = 70;
    int32_t limit_out = 0;
    battery_mngr.Init(); // Initialize manager, filling mode-limit map

    zassert_ok(battery_mngr.GetModeChargingLimit("MOBILE", limit_out));
    zassert_equal(limit_out, CHARGING_REL_CHG_STATE_DEFAULT);

    for(int i = 0; i < NUM_INSTALLATION_MODES; i++)
    {
        zassert_ok(battery_mngr.SetModeChargingLimit(installation_modes[i], limit_in));
        zassert_ok(battery_mngr.GetModeChargingLimit(installation_modes[i], limit_out));
        zassert_equal(limit_out, limit_in);
    }

    battery_mngr.StopSampling("GENERAL");
    battery_mngr.StopSampling("CHARGING");
}

ZTEST(battery_manager_suite, test_set_get_chg_limit_deny_unregistered_mode)
{
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    BatteryManager battery_mngr(test_bat_mock, test_chg);

    // Test setter/getter are denied when manager is not initialized to avoid corrupting limit-map.
    int32_t limit_in = 0xdead;
    int32_t limit_out = 0;
    for(int i = 0; i < NUM_INSTALLATION_MODES; i++)
    {
        zassert_equal(battery_mngr.SetModeChargingLimit(installation_modes[i], limit_in), EINVAL);
        zassert_equal(battery_mngr.GetModeChargingLimit(installation_modes[i], limit_out), EINVAL);
        zassert_equal(limit_out, 0);
    }

    battery_mngr.Init(); // Initialize manager, filling mode-limit map

    zassert_equal(battery_mngr.SetModeChargingLimit("UNKNOWN_MODE", limit_in), EINVAL);
    zassert_equal(battery_mngr.GetModeChargingLimit("UNKNOWN_MODE", limit_out), EINVAL);
    zassert_equal(limit_out, 0);

    battery_mngr.StopSampling("GENERAL");
    battery_mngr.StopSampling("CHARGING");
}

ZTEST(battery_manager_suite, test_initialized_set_get_chg_limit_outside_range)
{
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    BatteryManager battery_mngr(test_bat_mock, test_chg);

    int32_t limit_in = CHARGING_REL_CHG_STATE_MAX + 1;
    int32_t limit_out = 0;
    battery_mngr.Init(); // Initialize manager, filling mode-limit map
    for(int i = 0; i < NUM_INSTALLATION_MODES; i++)
    {
        zassert_equal(battery_mngr.SetModeChargingLimit(installation_modes[i], limit_in), ERANGE);
        zassert_ok(battery_mngr.GetModeChargingLimit(installation_modes[i], limit_out));
        zassert_equal(limit_out, CHARGING_REL_CHG_STATE_DEFAULT);
    }

    limit_in = CHARGING_REL_CHG_STATE_MIN - 1;
    limit_out = 0;
    battery_mngr.Init(); // Initialize manager, filling mode-limit map
    for(int i = 0; i < NUM_INSTALLATION_MODES; i++)
    {
        zassert_equal(battery_mngr.SetModeChargingLimit(installation_modes[i], limit_in), ERANGE);
        zassert_ok(battery_mngr.GetModeChargingLimit(installation_modes[i], limit_out));
        zassert_equal(limit_out, CHARGING_REL_CHG_STATE_DEFAULT);
    }

    battery_mngr.StopSampling("GENERAL");
    battery_mngr.StopSampling("CHARGING");
}

ZTEST(battery_manager_suite, test_adding_clearing_subscribers)
{
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    BatteryManager battery_mngr(test_bat_mock, test_chg);
    battery_mngr.Init();
    battery_mngr.StopSampling("GENERAL");
    battery_mngr.StopSampling("CHARGING");

    std::string test_sub = "CPU";
    std::string test_type = "GENERAL";

    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_ok(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_gen));
    zassert_true(battery_mngr.IsSubscribed(test_sub, test_type));
    battery_mngr.ClearSubscribers(test_type);
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));

    test_type = "CHARGING";
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_ok(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_chg));
    zassert_true(battery_mngr.IsSubscribed(test_sub, test_type));
    battery_mngr.ClearSubscribers(test_type);
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));
}

ZTEST(battery_manager_suite, test_subscriber_errors)
{
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    BatteryManager battery_mngr(test_bat_mock, test_chg);
    std::string test_sub = "CPU";

    // Subscriber is not registered as key in map
    std::string test_type = "GENERAL";
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_equal(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_gen), EINVAL);
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));

    test_type = "CHARGING";
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_equal(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_chg), EINVAL);
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));


    battery_mngr.Init();
    test_type = "GENERAL";
    // Subscriber is already subscribed
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_ok(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_gen));
    zassert_true(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_equal(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_gen), EEXIST);
    zassert_true(battery_mngr.IsSubscribed(test_sub, test_type));

    test_type = "CHARGING";
    zassert_false(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_ok(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_chg));
    zassert_true(battery_mngr.IsSubscribed(test_sub, test_type));
    zassert_equal(battery_mngr.AddSubscriber(test_sub, test_type, test_cb_chg), EEXIST);
    zassert_true(battery_mngr.IsSubscribed(test_sub, test_type));
}

ZTEST(battery_manager_suite, test_deny_low_sample_rate_chg_timer)
{
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    BatteryManager battery_mngr(test_bat_mock, test_chg);
    zassert_equal(battery_mngr.StartSampling("CHARGING", 20, CHARGING_PERIOD_UPPER_LIMIT_MSEC + 1), ERANGE);
    zassert_ok(battery_mngr.StartSampling("CHARGING", 20, CHARGING_PERIOD_UPPER_LIMIT_MSEC));
    battery_mngr.StopSampling("CHARGING");
}

ZTEST(battery_manager_suite, test_gen_cb_is_called_on_sample)
{
    std::string test_sub = "CPU";
    std::string test_type = "GENERAL";
    BatteryGeneralData test_data_veri;
    test_data_veri.cycle_count = 0;
    BatteryGeneralData test_data_exp;
    test_data_exp.cycle_count = 270;
    
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    test_bat_mock.SetTestCycleCount(test_data_exp.cycle_count);
    BatteryManager battery_mngr(test_bat_mock, test_chg);
    
    battery_mngr.Init();
    battery_mngr.StopSampling(test_type); // We need to stop sampling to do a more controlled unit test.

    battery_mngr.AddSubscriber(test_sub, test_type, test_cb_gen);
    zassert_true(battery_mngr.IsSubscribed(test_sub, test_type));

    zassert_equal(test_cb_gen(test_data_veri), 0);
    battery_mngr.StartSampling(test_type, 20, 60000);
    k_msleep(30);
    battery_mngr.StopSampling(test_type);
    battery_mngr.StopSampling("CHARGING");
    zassert_equal(test_cb_gen(test_data_veri), test_data_exp.cycle_count);
}

ZTEST(battery_manager_suite, test_chg_cb_is_called_on_sample)
{
    std::string test_sub = "CPU";
    std::string test_type = "CHARGING";
    BatteryChargingData test_data_veri;
    test_data_veri.des_chg_current= 0;
    BatteryChargingData test_data_exp;
    test_data_exp.des_chg_current = 5500;
    
    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    test_bat_mock.SetTestChargingCurrent(test_data_exp.des_chg_current);
    BatteryManager battery_mngr(test_bat_mock, test_chg);

    battery_mngr.Init();
    battery_mngr.StopSampling(test_type); // We need to stop sampling to do a more controlled unit test.

    battery_mngr.AddSubscriber(test_sub, test_type, test_cb_chg);

    zassert_equal(test_cb_chg(test_data_veri), 0);
    battery_mngr.StartSampling(test_type, 20, 3000);
    k_msleep(30);

    battery_mngr.StopSampling(test_type);
    battery_mngr.StopSampling("GENERAL");
    zassert_equal(test_cb_chg(test_data_veri), test_data_exp.des_chg_current);
}

ZTEST(battery_manager_suite, test_get_last_general_data)
{
    // Setup some dummy variables and assign them to the mocked battery
    BatteryGeneralData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.cycle_count = 120;

    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    test_bat_mock.SetTestTemp(test_data_exp.temp);
    test_bat_mock.SetTestVolt(test_data_exp.volt);
    test_bat_mock.SetTestCurrent(test_data_exp.current);
    test_bat_mock.SetTestRemCap(test_data_exp.remaining_capacity);
    test_bat_mock.SetTestCycleCount(test_data_exp.cycle_count);

    BatteryManager battery_mngr(test_bat_mock, test_chg);
    BatteryGeneralData test_data_res;
    zassert_equal(battery_mngr.GetLastData(test_data_res), EIO);
    zassert_equal(test_data_res.temp, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.volt, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.current, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.remaining_capacity, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.cycle_count, DEFAULT_INVALID_BAT_INT);

    battery_mngr.StartSampling("GENERAL", 20, 60000);
    k_msleep(30);

    zassert_ok(battery_mngr.GetLastData(test_data_res));
    battery_mngr.StopSampling("GENERAL");
    zassert_equal(test_data_res.temp, test_data_exp.temp);
    zassert_equal(test_data_res.volt, test_data_exp.volt);
    zassert_equal(test_data_res.current, test_data_exp.current);
    zassert_equal(test_data_res.remaining_capacity, test_data_exp.remaining_capacity);
    zassert_equal(test_data_res.cycle_count, test_data_exp.cycle_count);
};

ZTEST(battery_manager_suite, test_get_last_general_data_timer_stopped)
{
    // Setup some dummy variables and assign them to the mocked battery
    BatteryGeneralData test_data_exp;
    test_data_exp.temp = 20.0;
    test_data_exp.volt = 16.0;
    test_data_exp.current = 5.5;
    test_data_exp.remaining_capacity = 5000;
    test_data_exp.cycle_count = 120;

    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    test_bat_mock.SetTestTemp(test_data_exp.temp);
    test_bat_mock.SetTestVolt(test_data_exp.volt);
    test_bat_mock.SetTestCurrent(test_data_exp.current);
    test_bat_mock.SetTestRemCap(test_data_exp.remaining_capacity);
    test_bat_mock.SetTestCycleCount(test_data_exp.cycle_count);

    BatteryManager battery_mngr(test_bat_mock, test_chg);
    BatteryGeneralData test_data_res;
    zassert_equal(battery_mngr.GetLastData(test_data_res), EIO);
    zassert_equal(test_data_res.temp, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.volt, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.current, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.remaining_capacity, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.cycle_count, DEFAULT_INVALID_BAT_INT);

    battery_mngr.StartSampling("GENERAL", 20, 60000);
    k_msleep(30);

    battery_mngr.StopSampling("GENERAL");
    zassert_equal(battery_mngr.GetLastData(test_data_res), EIO);
    zassert_equal(test_data_res.temp, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.volt, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.current, DEFAULT_INVALID_BAT_FLOAT);
    zassert_equal(test_data_res.remaining_capacity, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.cycle_count, DEFAULT_INVALID_BAT_INT);
};

ZTEST(battery_manager_suite, test_get_last_chg_data)
{
    // Setup some dummy variables and assign them to the mocked battery
    BatteryChargingData test_data_exp;
    test_data_exp.des_chg_current = 4800;
    test_data_exp.des_chg_volt = 16500;
    test_data_exp.status = 192;
    test_data_exp.relative_charge_state = 80;

    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    test_bat_mock.SetTestChargingCurrent(test_data_exp.des_chg_current);
    test_bat_mock.SetTestChargingVoltage(test_data_exp.des_chg_volt);
    test_bat_mock.SetTestStatus(test_data_exp.status);
    test_bat_mock.SetTestRelChargeState(test_data_exp.relative_charge_state);
    BatteryManager battery_mngr(test_bat_mock, test_chg);
    BatteryChargingData test_data_res;
    zassert_equal(battery_mngr.GetLastData(test_data_res), EIO);
    zassert_equal(test_data_res.des_chg_current, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.des_chg_volt, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.status, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.relative_charge_state, DEFAULT_INVALID_BAT_INT);

    battery_mngr.StartSampling("CHARGING", 20, 3000);
    k_msleep(30);

    zassert_ok(battery_mngr.GetLastData(test_data_res));
    battery_mngr.StopSampling("CHARGING");
    zassert_equal(test_data_res.des_chg_current, test_data_exp.des_chg_current);
    zassert_equal(test_data_res.des_chg_volt, test_data_exp.des_chg_volt);
    zassert_equal(test_data_res.status, test_data_exp.status);
    zassert_equal(test_data_res.relative_charge_state, test_data_exp.relative_charge_state);
}

ZTEST(battery_manager_suite, test_get_last_chg_data_timer_stopped)
{
    // Setup some dummy variables and assign them to the mocked battery
    BatteryChargingData test_data_exp;
    test_data_exp.des_chg_current = 4800;
    test_data_exp.des_chg_volt = 16500;
    test_data_exp.status = 192;
    test_data_exp.relative_charge_state = 80;

    BatteryMock test_bat_mock;
    BatteryChargerMock test_chg;
    test_bat_mock.SetTestChargingCurrent(test_data_exp.des_chg_current);
    test_bat_mock.SetTestChargingVoltage(test_data_exp.des_chg_volt);
    test_bat_mock.SetTestStatus(test_data_exp.status);
    test_bat_mock.SetTestRelChargeState(test_data_exp.relative_charge_state);
    BatteryManager battery_mngr(test_bat_mock, test_chg);
    BatteryChargingData test_data_res;
    zassert_equal(battery_mngr.GetLastData(test_data_res), EIO);
    zassert_equal(test_data_res.des_chg_current, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.des_chg_volt, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.status, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.relative_charge_state, DEFAULT_INVALID_BAT_INT);

    battery_mngr.StartSampling("CHARGING", 20, 3000);
    k_msleep(30);

    battery_mngr.StopSampling("CHARGING");
    zassert_equal(battery_mngr.GetLastData(test_data_res), EIO);
    zassert_equal(test_data_res.des_chg_current, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.des_chg_volt, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.status, DEFAULT_INVALID_BAT_INT);
    zassert_equal(test_data_res.relative_charge_state, DEFAULT_INVALID_BAT_INT);
}