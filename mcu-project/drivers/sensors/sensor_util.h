#pragma once

#include <stdint.h>
#include <zephyr/drivers/sensor.h>

// Conversion functions
void i2c_bytes_to_sensor_value(uint8_t* buf, struct sensor_value* val);
void uint16_to_i2c_bytes(uint16_t val, uint8_t* buf);

#define Z_DEVICE_BASE_DEFINE_NO_INIT(node_id, dev_id, name, pm,                 \
                                     data, config, api, state, handles)         \
                                                                                \
    COND_CODE_1(DT_NODE_EXISTS(node_id), (), (static))                          \
                                                                                \
    const Z_DECL_ALIGN(struct device) DEVICE_NAME_GET(dev_id) Z_DEVICE_SECTION(APPLICATION, 1) __used \
        = Z_DEVICE_INIT(name, pm, data, config, api, state, handles)


#define Z_DEVICE_DEFINE_NO_INIT(node_id, dev_id, name, pm, data, config, api, state, ...) \
                                                                        \
	Z_DEVICE_NAME_CHECK(name);                                      \
                                                                        \
	Z_DEVICE_HANDLES_DEFINE(node_id, dev_id, __VA_ARGS__);          \
                                                                        \
	Z_DEVICE_BASE_DEFINE_NO_INIT(node_id, dev_id, name, pm,         \
        data, config, api, state, Z_DEVICE_HANDLES_NAME(dev_id));


#define DEVICE_DT_DEFINE_NO_INIT(node_id, pm_device, data, config, api, ...)    \
                                                                                \
    Z_DEVICE_STATE_DEFINE(Z_DEVICE_DT_DEV_ID(node_id));                         \
                                                                                \
    Z_DEVICE_DEFINE_NO_INIT(node_id, Z_DEVICE_DT_DEV_ID(node_id),               \
                        DEVICE_DT_NAME(node_id), pm, data, config,              \
                        api, &Z_DEVICE_STATE_NAME(Z_DEVICE_DT_DEV_ID(node_id)), \
			__VA_ARGS__)


#define DEVICE_DT_INST_DEFINE_NO_INIT(inst, ...) \
        DEVICE_DT_DEFINE_NO_INIT(DT_DRV_INST(inst), __VA_ARGS__)
