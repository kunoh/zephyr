#include "imu_mock.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(imu_mock, CONFIG_IMU_LOG_LEVEL);

ImuMock::ImuMock()
{}

int ImuMock::Init()
{
    LOG_INF("IMU Mock Init");
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