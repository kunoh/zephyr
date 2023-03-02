#pragma once
#include <inclinometer.h>

class InclinometerMock: public Inclinometer {
public:
    virtual ~InclinometerMock() = default;
    bool Write() override;
};