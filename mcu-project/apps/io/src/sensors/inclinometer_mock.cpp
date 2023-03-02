#include "inclinometer_mock.h"

bool InclinometerMock::Write()
{
    return true;
}

bool InclinometerMock::Read()
{
    return true;
}

void InclinometerMock::GetAngle(double xyz_angle[])
{}

InclinometerMock::InclinometerMock(Logger& logger) : logger_{logger}
{}