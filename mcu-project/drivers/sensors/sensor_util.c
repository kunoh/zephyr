#include "sensor_util.h"

// Conversion functions
void i2c_bytes_to_sensor_value(uint8_t* buf, struct sensor_value* val)
{
    val->val1 = ((int32_t)buf[1] << 8) | buf[0];
}

void uint16_to_i2c_bytes(uint16_t val, uint8_t* buf)
{
    buf[0] = val | 0x00ff;
	buf[1] = val | 0xff00;
}