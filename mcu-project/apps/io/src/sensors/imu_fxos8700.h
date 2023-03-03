#include <imu.h>
#include <logger.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

class ImuFxos8700 : public Imu {
public:
    ImuFxos8700(Logger& logger);
    virtual ~ImuFxos8700() = default;
    int FetchSampleData() override;
    void GetAccData(double& accx, double& accy, double& accz) override;
    void GetGyrData(double& gyrx, double& gyry, double& gyrz) override;
    void GetMagData(double& magx, double& magy, double& magz) override;
    void GetTempData(double& temp) override;
    int SetSamplesPerSecond(int integer_part, int fractional_part);
    int SetSlopeThreshold(int integer_part, int fractional_part);
    int SetTriggerOnDataReady(sensor_trigger_handler_t handler);
    int SetTriggerOnMotion(sensor_trigger_handler_t handler);

private:
    Logger& logger_;
    const device* imu_dev_;
    sensor_value accel_[3];
    sensor_value magn_[3];
    sensor_value temp_;
};