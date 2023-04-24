#include "nh2054qe34_driver.h"
#include <math.h>

#define DT_DRV_COMPAT inspiredenergy_nh2054qe34

static int read_battery_property(const struct i2c_dt_spec* i2c_spec, battery_command_t command, uint8_t *rx_buf)
{
    return i2c_burst_read(i2c_spec->bus, i2c_spec->addr, command, rx_buf, BATTERY_PROPERTY_BYTES);
}

static int write_battery_property(const struct i2c_dt_spec* i2c_spec, battery_command_t command, uint8_t *tx_buf)
{
    return i2c_burst_write(i2c_spec->bus, i2c_spec->addr, command, tx_buf, BATTERY_PROPERTY_BYTES);
}

// Configures battery and I2C controller on device initialization by Zephyr
static int nh2054qe34_init(const struct device *dev)
{   
    int ret;
    const struct nh2054qe34_config* battery_config = dev->config;
    const struct i2c_dt_spec battery_i2c_spec = battery_config->i2c_spec;
    struct nh2054qe34_data* battery_data = dev->data;
    uint32_t i2c_config = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER;
    
    // Configure I2C controller.
    ret = i2c_configure(battery_i2c_spec.bus, i2c_config);
    if(ret != 0)
    {
        return -EBUSY;
    }

    // Configure battery to report capacity in mAh
    ret = read_battery_property(&battery_i2c_spec, BATTERY_MODE, battery_data->mode);
    if(ret != 0)
    {
        return -EIO;
    }
    battery_data->mode[1] = (battery_data->mode[1] | BIT(6)) & BIT_MASK(7); // Set CAPACITY_MODE=0, CHARGER_MODE=1
    ret = write_battery_property(&battery_i2c_spec, BATTERY_MODE, battery_data->mode);
    if(ret != 0)
    {
        return -EIO;
    }

    return 0;
}

static int nh2054qe34_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    int ret = 0;
    const struct nh2054qe34_config* battery_config = dev->config;
    const struct i2c_dt_spec battery_i2c_spec = battery_config->i2c_spec;
    struct nh2054qe34_data* battery_data = dev->data;

    if(!device_is_ready(battery_i2c_spec.bus))
    {
        return -EINVAL;
    }

    if(chan == SENSOR_CHAN_ALL)
    {
        // Some ordering is important.
        // This is needed first needed to possibly scale voltage, current and capacity samples according to VScale and IPScale.
        ret = read_battery_property(&battery_i2c_spec, BATTERY_SPECIFICATION_INFO, battery_data->spec_info);
        if(ret != 0)
        {
            return -EIO;
        }

        ret = read_battery_property(&battery_i2c_spec, BATTERY_TEMPERATURE, battery_data->temp);
        if(ret != 0)
        {
            return -EIO;
        }
        ret = read_battery_property(&battery_i2c_spec, BATTERY_VOLTAGE, battery_data->volt);
        if(ret != 0)
        {
            return -EIO;
        }
        ret = read_battery_property(&battery_i2c_spec, BATTERY_CURRENT, battery_data->current);
        if(ret != 0)
        {
            return -EIO;
        }
        ret = read_battery_property(&battery_i2c_spec, BATTERY_REM_CAPACITY, battery_data->rem_cap);
        if(ret != 0)
        {
            return -EIO;
        }
        ret = read_battery_property(&battery_i2c_spec, BATTERY_STATUS, battery_data->status);
        if(ret != 0)
        {
            return -EIO;
        }
        ret = read_battery_property(&battery_i2c_spec, BATTERY_RELATIVE_STATE_OF_CHARGE, battery_data->relative_charge_state);
        if(ret != 0)
        {
            return -EIO;
        }
        ret = read_battery_property(&battery_i2c_spec, BATTERY_CYCLE_COUNT, battery_data->cycle_count);
        if(ret != 0)
        {
            return -EIO;
        }
    }

    return 0;
}

static int nh2054qe34_channel_get(const struct device* dev, enum sensor_channel chan, struct sensor_value* val)
{
    int ret = 0;
    struct nh2054qe34_data* battery_data = dev->data;
    
    switch (chan)
    {
        case SENSOR_CHAN_GAUGE_TEMP:;
            double temp = ((uint16_t)battery_data->temp[1] << 8) | battery_data->temp[0];
            sensor_value_from_double(val, temp);
            break;

        case SENSOR_CHAN_VOLTAGE:;
            double v_scale = pow(10, battery_data->spec_info[1] & 0x0f);
            double volt = ((double)(((uint16_t)battery_data->volt[1] << 8) | battery_data->volt[0]) / MILLIVOLTS_PER_VOLT) * v_scale;
            sensor_value_from_double(val, volt);
            break;
        
        case SENSOR_CHAN_CURRENT:;
            double i_scale = pow(10, (battery_data->spec_info[1] & 0xf0) >> 4);
            double current = ((double)(((int16_t)battery_data->current[1] << 8) | battery_data->current[0]) / MILLIAMPS_PER_AMP) * i_scale;
            sensor_value_from_double(val, current);
            break;

        case SENSOR_CHAN_GAUGE_REMAINING_CHARGE_CAPACITY:;
            double c_scale = pow(10, (battery_data->spec_info[1] & 0xf0) >> 4);
            val->val1 = (((int32_t)battery_data->rem_cap[1] << 8) | battery_data->rem_cap[0]) * (int32_t)(c_scale + 0.5);
            break;
        
        case SENSOR_CHAN_STATUS:
            val->val1 = ((int32_t)battery_data->status[1] << 8) | battery_data->status[0];
            break;

        case SENSOR_CHAN_GAUGE_STATE_OF_CHARGE:
            val->val1 = ((int32_t)battery_data->relative_charge_state[1] << 8) | battery_data->relative_charge_state[0];
            break;

        case SENSOR_CHAN_GAUGE_CYCLE_COUNT:
            val->val1 = ((int32_t)battery_data->cycle_count[1] << 8) | battery_data->cycle_count[0];
            break;
            
        default:
            ret = -EINVAL;
            break;
    }
    return ret;
}

static const struct sensor_driver_api nh2054qe34_driver_api = 
{
    .sample_fetch = nh2054qe34_sample_fetch,
    .channel_get = nh2054qe34_channel_get,
};

#define NH2054QE34_INIT(n)						\
	static const struct nh2054qe34_config nh2054qe34_config_##n = {	\
        .i2c_spec = I2C_DT_SPEC_INST_GET(n),        \
	};								\
									\
	static struct nh2054qe34_data nh2054qe34_data_##n; \
									\
	SENSOR_DEVICE_DT_INST_DEFINE(n,					\
			    nh2054qe34_init,				\
			    NULL,					\
			    &nh2054qe34_data_##n,				\
			    &nh2054qe34_config_##n,			\
			    POST_KERNEL,				\
			    CONFIG_SENSOR_INIT_PRIORITY,		\
			    &nh2054qe34_driver_api);

DT_INST_FOREACH_STATUS_OKAY(NH2054QE34_INIT);