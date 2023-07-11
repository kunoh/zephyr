#include "nh2054qe34_driver.h"
#include "sensor_util.h"

#include <stdio.h>
#include <math.h>

#define DT_DRV_COMPAT inspiredenergy_nh2054qe34

// I2C
static int i2c_battery_config(const struct i2c_dt_spec* i2c_spec)
{
    // Configure I2C controller.
    uint32_t i2c_config = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER;	// 100 kHz

    if(i2c_configure(i2c_spec->bus, i2c_config) != 0)
    {
        return EINVAL;
    }

    return 0;
}

// I2C
static int read_battery_property(const struct i2c_dt_spec* i2c_spec, battery_command_t command, uint8_t *rx_buf)
{
    return i2c_burst_read(i2c_spec->bus, i2c_spec->addr, command, rx_buf, BATTERY_PROPERTY_BYTES);
}

static int write_battery_property(const struct i2c_dt_spec* i2c_spec, battery_command_t command, uint8_t *tx_buf)
{
    return i2c_burst_write(i2c_spec->bus, i2c_spec->addr, command, tx_buf, BATTERY_PROPERTY_BYTES);
}

static int nh2054qe34_init(const struct device* dev, const struct nh2054qe34_config* battery_config, const struct i2c_dt_spec battery_i2c_spec, struct nh2054qe34_data* battery_data)
{
    int ret = 0;

    ret = read_battery_property(&battery_i2c_spec, BATTERY_MODE, battery_data->mode);
    if(ret != 0)
    {
        return EIO;
    }
    // Set:
    // CAPACITY_MODE = 0    (Report in Ah or mAh)
    // CHARGER_MODE = 1     (Disable broadcasts of charging voltage and current)
    // ALARM_MODE = 1       (Disable battery alarm broadcasts)
    uint8_t mode_to_write[2];
    mode_to_write[0] = battery_data->mode[0];
    mode_to_write[1] = (battery_data->mode[1]
                            | BIT(BATTERY_MODE_ALARM_BIT_POS)
                            | BIT(BATTERY_MODE_CHARGER_BIT_POS))
                            & BIT_MASK(BATTERY_MODE_CAPACITY_BIT_POS);
    ret = write_battery_property(&battery_i2c_spec, BATTERY_MODE, mode_to_write);
    if(ret != 0)
    {
        return EIO;
    }
    
    k_msleep(1);
    // Validate
    ret = read_battery_property(&battery_i2c_spec, BATTERY_MODE, battery_data->mode);
    if(ret != 0 || battery_data->mode[0] != mode_to_write[0] || battery_data->mode[1] != mode_to_write[1])
    {
        return EIO;
    }

    return ret;
}

static void conv_temp(uint8_t* buf, struct sensor_value* val)
{
    // Kelvin
    double temp = ((uint16_t)buf[1] << 8) | buf[0];
    sensor_value_from_double(val, temp);
}

static void conv_volt(uint8_t* buf, struct sensor_value* val, uint8_t* spec_info_buf)
{
    // Volts
    double v_scale = pow(10, spec_info_buf[1] & BIT_MASK_VOLTAGE_SCALING);
    double volt = ((double)(((uint16_t)buf[1] << 8) | buf[0]) / MILLIVOLTS_PER_VOLT) * v_scale;
    sensor_value_from_double(val, volt);
}

static void conv_current(uint8_t* buf, struct sensor_value* val, uint8_t* spec_info_buf)
{
    // Amps
    double i_scale = pow(10, (spec_info_buf[1] & BIT_MASK_CURRENT_SCALING) >> 4);
    double current = ((double)(((int16_t)buf[1] << 8) | buf[0]) / MILLIAMPS_PER_AMP) * i_scale;
    sensor_value_from_double(val, current);
}

static void conv_rem_charge_cap(uint8_t* buf, struct sensor_value* val, uint8_t* spec_info_buf)
{
    // mAh
    double c_scale = pow(10, (spec_info_buf[1] & BIT_MASK_CAPACITY_SCALING) >> 4);
    val->val1 = (((int32_t)buf[1] << 8) | buf[0]) * (int32_t)(c_scale + 0.5);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wenum-compare"
static int nh2054qe34_attr_set(const struct device *dev,
                 enum sensor_channel chan,
                 enum sensor_attribute attr,
                 const struct sensor_value *val)
{
    int ret = EINVAL;
    const struct nh2054qe34_config* battery_config = dev->config;
    const struct i2c_dt_spec battery_i2c_spec = battery_config->i2c_spec;
    struct nh2054qe34_data* battery_data = dev->data;

    // Configure I2C controller.
    ret = i2c_battery_config(&battery_i2c_spec);
    if(ret != 0)
    {
        return EBUSY;
    }

    if(!i2c_is_ready_dt(&battery_i2c_spec))
    {
        return EBUSY;
    }

    if(attr == SENSOR_ATTR_CONFIGURATION && chan == SENSOR_CHAN_INIT_CONFIG)
    {
        ret = nh2054qe34_init(dev, battery_config, battery_i2c_spec, battery_data);
        mark_device_cond_initialized(dev, ret);
    }

    return ret;
}
#pragma GCC diagnostic pop

// Private extensions of the sensor API's sensor_channel enum are needed for missing sensor property types.
// When used in a switch or if statement on a value of type sensor_channel these extensions produce warnings which we suppress.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
static int nh2054qe34_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    int ret = 0;
    
    const struct nh2054qe34_config* battery_config = dev->config;
    const struct i2c_dt_spec battery_i2c_spec = battery_config->i2c_spec;
    struct nh2054qe34_data* battery_data = dev->data;

     // Configure I2C controller.
    ret = i2c_battery_config(&battery_i2c_spec);
    if(ret != 0)
    {
        return EBUSY;
    }

    if(!i2c_is_ready_dt(&battery_i2c_spec))
    {
        return EBUSY;
    }

    switch(chan)
    {
        case SENSOR_CHAN_BATTERY_GENERAL:
            // Some ordering is important.
            // spec_info is needed first needed to possibly scale voltage, current and capacity samples according to VScale and IPScale.
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_SPECIFICATION_INFO, battery_data->spec_info);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_TEMPERATURE, battery_data->temp);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_VOLTAGE, battery_data->volt);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_CURRENT, battery_data->current);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_REM_CAPACITY, battery_data->rem_cap);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_CYCLE_COUNT, battery_data->cycle_count);
            break;

        case SENSOR_CHAN_BATTERY_CHARGING:
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_CHARGING_CURRENT, battery_data->charging_current);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_CHARGING_VOLTAGE, battery_data->charging_volt);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_STATUS, battery_data->status);
            ret |= read_battery_property(&battery_i2c_spec, BATTERY_RELATIVE_STATE_OF_CHARGE, battery_data->relative_charge_state);
            break;

        default:
            return EINVAL;
            break;
    }

    return ret;
}


static int nh2054qe34_channel_get(const struct device* dev, enum sensor_channel chan, struct sensor_value* val)
{
    int ret = 0;
    struct nh2054qe34_data* battery_data = dev->data;
    
    switch (chan)
    {
        case SENSOR_CHAN_BATTERY_GENERAL:
            conv_temp(battery_data->temp, val);
            conv_current(battery_data->current, &val[1], battery_data->spec_info);
            conv_volt(battery_data->volt, &val[2], battery_data->spec_info);
            conv_rem_charge_cap(battery_data->rem_cap, &val[3], battery_data->spec_info);
            i2c_bytes_to_sensor_value(battery_data->cycle_count, &val[4]);
            break;

        case SENSOR_CHAN_BATTERY_CHARGING:
            // mAmps both ways. NOT scaled by IPScale.
            i2c_bytes_to_sensor_value(battery_data->charging_current, val);
            // mVolts both ways. NOT scaled by VScale.
            i2c_bytes_to_sensor_value(battery_data->charging_volt, &val[1]);
            i2c_bytes_to_sensor_value(battery_data->status, &val[2]);
            i2c_bytes_to_sensor_value(battery_data->relative_charge_state, &val[3]);
            break;

        case SENSOR_CHAN_GAUGE_TEMP:;
            conv_temp(battery_data->temp, val);
            break;

        case SENSOR_CHAN_VOLTAGE:;
            conv_volt(battery_data->volt, val, battery_data->spec_info);
            break;
        
        case SENSOR_CHAN_CURRENT:;
            conv_current(battery_data->current, val, battery_data->spec_info);
            break;

        case SENSOR_CHAN_GAUGE_REMAINING_CHARGE_CAPACITY:;
            conv_rem_charge_cap(battery_data->rem_cap, val, battery_data->spec_info);
            break;

        case SENSOR_CHAN_GAUGE_STATE_OF_CHARGE:
            i2c_bytes_to_sensor_value(battery_data->relative_charge_state, val);
            break;

        case SENSOR_CHAN_GAUGE_CYCLE_COUNT:
            i2c_bytes_to_sensor_value(battery_data->cycle_count, val);
            break;

        case SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT:
            // mAmps both ways. NOT scaled by IPScale.
            i2c_bytes_to_sensor_value(battery_data->charging_current, val);
            break;
        
        case SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE:
            // mVolts both ways. NOT scaled by VScale.
            i2c_bytes_to_sensor_value(battery_data->charging_volt, val);
            break;
        
        case SENSOR_CHAN_STATUS:
            i2c_bytes_to_sensor_value(battery_data->status, val);
            break;
            
        default:
            ret = EINVAL;
            break;
    }
    return ret;
}

#pragma GCC diagnostic pop

static const struct sensor_driver_api nh2054qe34_driver_api = 
{
    .attr_set = nh2054qe34_attr_set,
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
    DEVICE_DT_INST_DEFINE_NO_INIT(n,        \
                                    NULL,   \
                                    &nh2054qe34_data_##n, \
                                    &nh2054qe34_config_##n, \
                                    &nh2054qe34_driver_api);

DT_INST_FOREACH_STATUS_OKAY(NH2054QE34_INIT);