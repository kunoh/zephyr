#pragma once
#include <imu.h>
#include <logger.h>

class ImuMock : public Imu {
public:
    ImuMock(Logger& logger);
    virtual ~ImuMock() = default;
    int FetchSampleData() override;
    void GetAccData(double& accx, double& accy, double& accz) override;
    void GetGyrData(double& gyrx, double& gyry, double& gyrz) override;
    void GetMagData(double& magx, double& magy, double& magz) override;
    void GetTempData(double& temp) override;

private:
    Logger& logger_;
};