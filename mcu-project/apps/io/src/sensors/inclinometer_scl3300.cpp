#include "inclinometer_scl3300.h"

#include <zephyr/logging/log.h>

#include "scl3300_driver.h"
#include "util.h"

LOG_MODULE_REGISTER(incl_scl3300, CONFIG_INCLINOMETER_LOG_LEVEL);

InclinometerScl3300::InclinometerScl3300()
{}

int InclinometerScl3300::Init()
{
    LOG_INF("Inclinometer Scl3300 Init");

    if (!device_is_ready(incl_dev_)) {
        LOG_ERR("Inclinometer not found. Aborting...\r\n");
        return 1;
    }

    return 0;
}

bool InclinometerScl3300::Write()
{
    LOG_WRN("InclinometerScl3300::Write() Not yet implemented");
    return true;
}

bool InclinometerScl3300::Read()
{
    int status = 0;
    struct sensor_value temp_buffer_array[3];

    status = sensor_sample_fetch(incl_dev_);
    if (status) {
        LOG_ERR("Could not fetch inclinometer sample data");
    }

    status |= sensor_channel_get(incl_dev_, SENSOR_CHAN_ACCEL_XYZ, temp_buffer_array);
    acc_x_ = sensor_value_to_double(&temp_buffer_array[0]);
    acc_y_ = sensor_value_to_double(&temp_buffer_array[1]);
    acc_z_ = sensor_value_to_double(&temp_buffer_array[2]);

    status |=
        sensor_channel_get(incl_dev_, (sensor_channel)SENSOR_CHAN_ANGLE_XYZ, temp_buffer_array);
    angle_x_ = sensor_value_to_double(&temp_buffer_array[0]);
    angle_y_ = sensor_value_to_double(&temp_buffer_array[1]);
    angle_z_ = sensor_value_to_double(&temp_buffer_array[2]);

    return status;
}

void InclinometerScl3300::GetAngle(SensorSampleData *xyz_angle)
{
    xyz_angle->x = angle_x_;
    xyz_angle->y = angle_y_;
    xyz_angle->z = angle_z_;
}
