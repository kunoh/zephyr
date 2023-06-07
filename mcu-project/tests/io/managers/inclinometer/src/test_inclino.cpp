#include <zephyr/ztest.h> // https://github.com/zephyrproject-rtos/zephyr/issues/55450
#include <zephyr/kernel.h>
#include <stdio.h>
#include "util.h"
#include "logger_zephyr.h"
#include "inclinometer_manager.h"
#include "inclinometer_mock.h"


ZTEST_SUITE(inclino_manager_suite, NULL, NULL, NULL, NULL, NULL);


// Test function to check subscribtions.
int testCallBack(uint32_t test){
    static int testVar = 0;

    testVar += test;
    return testVar;
}

//
//  Test adding subscribers to inclinometer manager.
//
ZTEST(inclino_manager_suite, test_subscriber)
{
    // Setup
    LoggerZephyr logger("");
    InclinometerMock incl(logger);
    InclinometerManager inclino_manager(logger, incl);

    zassert_equal(inclino_manager.GetSubscribeCount(), 0);

    // Test sub
    inclino_manager.Subscribe(testCallBack);
    zassert_equal(inclino_manager.GetSubscribeCount(), 1);

    // Test subcribtion is called.

    zassert_equal(testCallBack(0), 0);
    inclino_manager.StartInclinoTimer();
    k_sleep(K_MSEC(2200));
    inclino_manager.StopInclinoTimer();
    zassert_true(testCallBack(0) != 0);

};


//
// Test that manager data collection works.
//
ZTEST(inclino_manager_suite, test_collect_data)
{
    // Setup
    LoggerZephyr logger("");
    InclinometerMock incl(logger);
    InclinometerManager inclino_manager(logger, incl);

    double last_known_x_val;
    double last_known_y_val;
    double last_known_z_val;
    double second_last_known_x_val;
    double second_last_known_y_val;
    double second_last_known_z_val;

    // Test
    //

    // verify that internal vars are 0 at startup.
    last_known_x_val = inclino_manager.GetLastXAngle();
    last_known_y_val = inclino_manager.GetLastYAngle();
    last_known_z_val = inclino_manager.GetLastZAngle();
    zassert_equal(last_known_x_val, 0, "X angle NOT 0 at init!");
    zassert_equal(last_known_y_val, 0, "Y angle NOT 0 at init!");
    zassert_equal(last_known_z_val, 0, "Z angle NOT 0 at init!");

    // Check internal vars change when new data is read.
    inclino_manager.StartInclinoTimer();
    k_sleep(K_MSEC(2200));
    last_known_x_val = inclino_manager.GetLastXAngle();
    last_known_y_val = inclino_manager.GetLastYAngle();
    last_known_z_val = inclino_manager.GetLastZAngle();
    zassert_true(last_known_x_val != 0, "X angle equal 0 after read!");
    zassert_true(last_known_y_val != 0, "Y angle equal 0 after read!");
    zassert_true(last_known_z_val != 0, "Z angle equal 0 after read!");

    // Verify that values keep changeing when new data is read.
    k_sleep(K_MSEC(2200));
    inclino_manager.StopInclinoTimer();
    second_last_known_x_val = inclino_manager.GetLastXAngle();
    second_last_known_y_val = inclino_manager.GetLastYAngle();
    second_last_known_z_val = inclino_manager.GetLastZAngle();
    zassert_true(last_known_x_val != second_last_known_x_val, "X angle did not change after read!");
    zassert_true(last_known_y_val != second_last_known_y_val, "Y angle did not change after read!");
    zassert_true(last_known_z_val != second_last_known_z_val, "Z angle did not change after read!");

};
