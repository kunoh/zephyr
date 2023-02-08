#pragma once
#include "sensor.h"

class Display: public Sensor {
public:
    bool write();
};