#pragma once
#include "sensor.h"

class Inclinometer: public Sensor {
public:
    bool write();
};