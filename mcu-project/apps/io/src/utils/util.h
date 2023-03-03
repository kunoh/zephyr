#pragma once

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