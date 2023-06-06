#pragma once

#include <stdint.h>
#include <zephyr/drivers/sensor.h>

// Conversion functions
void i2c_bytes_to_sensor_value(uint8_t* buf, struct sensor_value* val);
void uint16_to_i2c_bytes(uint16_t val, uint8_t* buf);
