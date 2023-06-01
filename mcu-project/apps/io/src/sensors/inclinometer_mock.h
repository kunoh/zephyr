#pragma once
#include <stdio.h>

#include <cstdlib>

#include "inclinometer.h"
#include "logger.h"

class InclinometerMock : public Inclinometer {
public:
    InclinometerMock(Logger& logger);
    virtual ~InclinometerMock() = default;
    bool Write() override;
    bool Read() override;
    void GetAngle(double xyz_angle[]) override;

private:
    Logger& logger_;
};