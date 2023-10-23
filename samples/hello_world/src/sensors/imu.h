#pragma once

class Imu {
public:
    virtual ~Imu() = default;
    virtual int Init() = 0;
    virtual int FetchSampleData() = 0;
    virtual void GetAccData(double& accx, double& accy, double& accz) = 0;
    virtual void GetGyrData(double& gyrx, double& gyry, double& gyrz) = 0;
    virtual void GetMagData(double& magx, double& magy, double& magz) = 0;
    virtual void GetTempData(double& temp) = 0;
};