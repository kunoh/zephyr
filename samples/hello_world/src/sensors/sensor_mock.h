#pragma once
#include <sensor.h>


class SensorMock : public Sensor {
public:
    SensorMock();
    virtual ~SensorMock() = default;
    int Init() override;
    int FetchSampleData() override;
    void GetSensorValue(float &val) override;
    void GetSensorValue2(float &val) override;
    void GetSensorValue3(float &val) override;
    void GetSensorValue4(int &val) override;
};