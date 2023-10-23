#pragma once

class Sensor {
public:
    virtual ~Sensor() = default;
    virtual int Init() = 0;
    virtual int FetchSampleData() = 0;
    virtual void GetSensorValue(float &val) = 0;
    virtual void GetSensorValue2(float &val) = 0;
    virtual void GetSensorValue3(float &val) = 0;
    virtual void GetSensorValue4(int &val) = 0;
};