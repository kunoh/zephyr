#pragma once
#include <inclinometer.h>

class Inclinometer {
public:
    virtual ~Inclinometer() = default;
    virtual bool write() = 0;
};