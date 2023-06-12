#pragma once
#include <zephyr/logging/log.h>

#include "imu.h"

class ImuMock : public Imu {
public:
    ImuMock();
    virtual ~ImuMock() = default;
    int Init() override;
    int FetchSampleData() override;
    void GetAccData(double& accx, double& accy, double& accz) override;
    void GetGyrData(double& gyrx, double& gyry, double& gyrz) override;
    void GetMagData(double& magx, double& magy, double& magz) override;
    void GetTempData(double& temp) override;

private:
};