#include "inclinometer_scl3300.h"

#include "scl3300_driver.h"

bool Inclinometer_scl3300::Write()
{
    return true;
}

bool Inclinometer_scl3300::Read()
{
    int status = 0;
    struct sensor_value temp_buffer_array[3];

    status = sensor_sample_fetch(incl_dev_);
    if (status) {
        logger_.err("Could not fetch inclinometer sample data");
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

    return true;
}

void Inclinometer_scl3300::GetAngle(double xyz_angle[])
{
    xyz_angle[0] = angle_x_;
    xyz_angle[1] = angle_y_;
    xyz_angle[2] = angle_z_;
}

Inclinometer_scl3300::Inclinometer_scl3300(Logger& logger) : logger_{logger}
{
    printf("Inclinometer_scl3300::Inclinometer_scl3300\r\n");

    if (!device_is_ready(incl_dev_)) {
        printk("Inclinometer not found. Aborting...\r\n");
    }
}