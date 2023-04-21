#pragma once
#include <stdint.h>
#include <zephyr/kernel.h>

struct CbWrapper {
    void *user_data;
    void (*cb)(void*);
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

struct BatteryData {
    float temp;
    float current;
    float volt;
    uint32_t remaining_capacity;
    uint16_t status;
    uint8_t relative_charge_state;
    uint16_t cycle_count;
};