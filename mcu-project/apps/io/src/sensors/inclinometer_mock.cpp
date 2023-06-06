#include "inclinometer_mock.h"

bool InclinometerMock::Write()
{
    return true;
}

bool InclinometerMock::Read()
{
    // put som random numbers in the correct ranges into the holding vars.
    acc_x_ = (rand() % 100) / 100.0;
    acc_y_ = (rand() % 100) / 100.0;
    acc_z_ = (rand() % 100) / 100.0;

    angle_x_ = (rand() % 3600) / 10.0;
    angle_y_ = (rand() % 3600) / 10.0;
    angle_z_ = (rand() % 3600) / 10.0;
    return true;
}

void InclinometerMock::GetAngle(double xyz_angle[])
{
    xyz_angle[0] = angle_x_;
    xyz_angle[1] = angle_y_;
    xyz_angle[2] = angle_z_;
}

InclinometerMock::InclinometerMock(Logger& logger) : logger_{logger}
{
    acc_x_ = 0;
    acc_y_ = 0;
    acc_z_ = 0;

    angle_x_ = 0;
    angle_y_ = 0;
    angle_z_ = 0;

    // init random number function.
    // initialize random seed:
    srand(1);
}
