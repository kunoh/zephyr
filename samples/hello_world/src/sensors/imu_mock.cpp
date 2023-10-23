#include "imu_mock.h"

#include "zephyr/sys/printk.h"

ImuMock::ImuMock()
{}

int ImuMock::Init()
{
    printk("ImuMock Init\n");
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