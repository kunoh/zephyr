#pragma once
#include <cstdint>

class Inclinometer {
public:
    virtual ~Inclinometer() = default;
    virtual int Init() = 0;
    virtual bool Write() = 0;
    virtual bool Read() = 0;
    virtual void GetAngle(double XYZAngle[]) = 0;
};