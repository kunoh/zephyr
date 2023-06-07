#include "imu_fxos8700.h"

ImuFxos8700::ImuFxos8700(Logger& logger) : logger_{logger}
{}

int ImuFxos8700::Init()
{
    logger_.inf("IMU FXOS8700 Init");

    imu_dev_ = device_get_binding(DT_LABEL(DT_INST(0, nxp_fxos8700)));

    if (!device_is_ready(imu_dev_)) {
        logger_.err("ImuFxos8700 not found. Aborting...");
        return 1;
    }

    return SetSamplesPerSecond(6, 250000);
}

int ImuFxos8700::SetSamplesPerSecond(int integer_part, int fractional_part)
{
    sensor_value attr = {
        .val1 = integer_part,
        .val2 = fractional_part,
    };

    int ret = sensor_attr_set(imu_dev_, SENSOR_CHAN_ALL, SENSOR_ATTR_SAMPLING_FREQUENCY, &attr);
    if (ret) {
        logger_.err("Could not set sampling frequency");
    }
    return ret;
}

int ImuFxos8700::SetSlopeThreshold(int integer_part, int fractional_part)
{
    sensor_value attr = {
        .val1 = integer_part,
        .val2 = fractional_part,
    };

    int ret = sensor_attr_set(imu_dev_, SENSOR_CHAN_ALL, SENSOR_ATTR_SLOPE_TH, &attr);
    if (ret) {
        logger_.err("Could not set slope threshold");
    }
    return ret;
}

int ImuFxos8700::SetTriggerOnDataReady(sensor_trigger_handler_t handler)
{
    sensor_trigger trig = {
        .type = SENSOR_TRIG_DATA_READY,
        .chan = SENSOR_CHAN_ACCEL_XYZ,
    };

    int ret = sensor_trigger_set(imu_dev_, &trig, handler);
    if (ret) {
        logger_.err("Could not set trigger");
    }
    return ret;
}

int ImuFxos8700::SetTriggerOnMotion(sensor_trigger_handler_t handler)
{
    sensor_trigger trig = {
        .type = SENSOR_TRIG_DELTA,
        .chan = SENSOR_CHAN_ACCEL_XYZ,
    };

    int ret = sensor_trigger_set(imu_dev_, &trig, handler);
    if (ret) {
        logger_.err("Could not set trigger");
    }
    return ret;
}

int ImuFxos8700::FetchSampleData()
{
    int ret = sensor_sample_fetch(imu_dev_);
    if (ret) {
        logger_.err("Could not fetch sample data");
    }
    ret |= sensor_channel_get(imu_dev_, SENSOR_CHAN_ACCEL_XYZ, accel_);
    if (ret) {
        logger_.err("Could not get accelerometer data");
    }
    ret |= sensor_channel_get(imu_dev_, SENSOR_CHAN_MAGN_XYZ, magn_);
    if (ret) {
        logger_.err("Could not get magnometer data");
    }
    ret |= sensor_channel_get(imu_dev_, SENSOR_CHAN_DIE_TEMP, &temp_);
    if (ret) {
        logger_.err("Could not get temperature");
    }
    return ret;
}

void ImuFxos8700::GetAccData(double& accx, double& accy, double& accz)
{
    accx = sensor_value_to_double(&accel_[0]);
    accy = sensor_value_to_double(&accel_[1]);
    accz = sensor_value_to_double(&accel_[2]);
}

void ImuFxos8700::GetGyrData(double& gyrx, double& gyry, double& gyrz)
{}

void ImuFxos8700::GetMagData(double& magx, double& magy, double& magz)
{
    magx = sensor_value_to_double(&magn_[0]);
    magy = sensor_value_to_double(&magn_[1]);
    magz = sensor_value_to_double(&magn_[2]);
}

void ImuFxos8700::GetTempData(double& temp)
{
    temp = sensor_value_to_double(&temp_);
}
