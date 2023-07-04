#pragma once
#include <float.h>
#include <stdint.h>
#include <zephyr/kernel.h>

#include <functional>
#include <string>

#define DEFAULT_INVALID_BAT_INT INT32_MIN
#define DEFAULT_INVALID_BAT_FLOAT -1000.0

#define NUM_INSTALLATION_MODES 2
#define NUM_SUBSCRIBER_TYPES 1
const std::string installation_modes[NUM_INSTALLATION_MODES] = {"MOBILE", "FIXED"};
const std::string subscriber_types[NUM_SUBSCRIBER_TYPES] = {"CPU"};

struct CallbackWrapper {
    void *user_data = NULL;
    void (*cb)(void *) = NULL;
};

struct SensorSampleData {
    double x;
    double y;
    double z;
};

struct ImuSampleData {
    SensorSampleData acc{.x = 0, .y = 0, .z = 0};
    SensorSampleData gyr{.x = 0, .y = 0, .z = 0};
    SensorSampleData mag{.x = 0, .y = 0, .z = 0};
};
struct BatteryGeneralData {
    float temp = DEFAULT_INVALID_BAT_FLOAT;
    float current = DEFAULT_INVALID_BAT_FLOAT;
    float volt = DEFAULT_INVALID_BAT_FLOAT;
    int32_t remaining_capacity = DEFAULT_INVALID_BAT_INT;
    int32_t cycle_count = DEFAULT_INVALID_BAT_INT;
};

struct BatteryChargingData {
    int32_t des_chg_current = DEFAULT_INVALID_BAT_INT;
    int32_t des_chg_volt = DEFAULT_INVALID_BAT_INT;
    int32_t status = DEFAULT_INVALID_BAT_INT;
    int32_t relative_charge_state = DEFAULT_INVALID_BAT_INT;
    bool charging = false;
};

struct GeneralSubscription {
    std::function<int(BatteryGeneralData)> cb = NULL;
    BatteryGeneralData thresholds = {
        .temp = 0.1,
        .current = 0.1,
        .volt = 0.1,
        .remaining_capacity = 1,
        .cycle_count = 1};  // We use a change-in-value from last notification threshold to
                            // determine when to push a new notification.
    BatteryGeneralData last_notificaton_;
};

struct ChargingSubscription {
    std::function<int(BatteryChargingData)> cb = NULL;
    BatteryChargingData thresholds = {
        .des_chg_current = 1,
        .des_chg_volt = 1,
        .relative_charge_state = 1};  // We use a change-in-value from last notification threshold
                                      // to determine when to push a new notification.
    BatteryChargingData last_notification_;
};