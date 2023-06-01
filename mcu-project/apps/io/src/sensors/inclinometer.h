#pragma once
#include <cstdint>

class Inclinometer {
public:
    virtual ~Inclinometer() = default;
    virtual bool Write() = 0;
    virtual bool Read() = 0;
    virtual void GetAngle(double XYZAngle[]) = 0;
};