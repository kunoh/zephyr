#pragma once
#include <float.h>
#include <stdint.h>
#include <zephyr/kernel.h>

#define DEFAULT_INVALID_BAT_INT INT32_MIN
#define DEFAULT_INVALID_BAT_FLOAT -1000.0

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
