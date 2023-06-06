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
    double acc_x_;
    double acc_y_;
    double acc_z_;
    double angle_x_;
    double angle_y_;
    double angle_z_;
};