#include "sensor_mock.h"

#include "zephyr/sys/printk.h"

SensorMock::SensorMock()
{}

int SensorMock::Init()
{
    printk("SensorMock Init\n");
    return 0;
}

int SensorMock::FetchSampleData()
{
    return 0;
}

void SensorMock::GetSensorValue(float &val)
{}

void SensorMock::GetSensorValue2(float &val)
{}

void SensorMock::GetSensorValue3(float &val)
{}

void SensorMock::GetSensorValue4(int &val)
{}