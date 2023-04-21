#include "imu_mock.h"

ImuMock::ImuMock(Logger& logger) : logger_{logger}
{}

int ImuMock::Init()
{
    logger_.inf("IMU Mock Init");
    return 0;
}

int ImuMock::FetchSampleData()
{
    return 0;
}

void ImuMock::GetAccData(double& accx, double& accy, double& accz)
{}

void ImuMock::GetGyrData(double& gyrx, double& gyry, double& gyrz)
{}

void ImuMock::GetMagData(double& magx, double& magy, double& magz)
{}

void ImuMock::GetTempData(double& temp)
{}