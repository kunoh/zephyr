#include "sensor_util.h"

// Conversion functions
void i2c_bytes_to_sensor_value(uint8_t* buf, struct sensor_value* val)
{
    val->val1 = ((int32_t)buf[1] << 8) | buf[0];
}

void uint16_to_i2c_bytes(uint16_t val, uint8_t* buf)
{
    buf[0] = (uint8_t)(val & 0x00ff);
	buf[1] = (uint8_t)(val >> 8);
}

void mark_device_cond_initialized(const struct device* dev, int init_rc)
{
    if (dev != NULL) {
        if (init_rc != 0) {
            if (init_rc < 0) {
                init_rc = -init_rc;
            }
            if (init_rc > UINT8_MAX) {
                init_rc = UINT8_MAX;
            }
            dev->state->init_res = init_rc;
        }
        else
        {
            dev->state->init_res = init_rc;
            dev->state->initialized = true;
        }
	}
}