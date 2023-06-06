#include "bq25713_driver.h"
#include "sensor_util.h"

#include <stdio.h>

#define DT_DRV_COMPAT ti_bq25713
    
// I2C
static int i2c_charger_config(const struct i2c_dt_spec* i2c_spec)
{
	// Configure I2C controller.
	uint32_t i2c_config = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_CONTROLLER;	// 400 kHz

    if(i2c_configure(i2c_spec->bus, i2c_config) != 0)
    {
        return EINVAL;
    }

    if(!device_is_ready(i2c_spec->bus))
    {
        return EBUSY;
    }

	return 0;
}

static int set_charging_current(const struct i2c_dt_spec* charger_i2c_spec, const struct sensor_value* val)
{
	int ret = 0;
	uint8_t tx_buf[2];
	uint16_t chg_current = ((uint16_t)val->val1) & BQ25713_CURRENT_REG_BIT_MASK;
	if(chg_current > 0 && (chg_current < BQ25713_CURRENT_MIN || chg_current > BQ25713_CURRENT_MAX))
	{
		return ERANGE;
	} 

	uint16_to_i2c_bytes(chg_current, tx_buf);
	ret = i2c_burst_write(charger_i2c_spec->bus, charger_i2c_spec->addr, CHARGER_CHARGING_CURRENT, tx_buf, CHARGER_PROPERTY_BYTES);
	if(ret != 0)
	{
		ret = EIO;
	}

	return ret;
}

static int set_charging_voltage(const struct i2c_dt_spec* charger_i2c_spec, const struct sensor_value* val)
{
	int ret = 0;
	uint8_t tx_buf[2];
	uint16_t chg_volt = ((uint16_t)val->val1) & BQ25713_VOLTAGE_REG_BIT_MASK;
	if(chg_volt > 0 && (chg_volt < BQ25713_VOLT_MIN || chg_volt > BQ25713_4_S_VOLT_MAX))
	{
		return ERANGE;
	}

	uint16_to_i2c_bytes(chg_volt, tx_buf);
	ret = i2c_burst_write(charger_i2c_spec->bus, charger_i2c_spec->addr, CHARGER_CHARGING_VOLTAGE, tx_buf, CHARGER_PROPERTY_BYTES);
	if(ret != 0)
	{
		ret = EIO;
	}
	return ret;
}

static int get_charging_current(const struct i2c_dt_spec* charger_i2c_spec, struct bq25713_data* charger_data, struct sensor_value* val)
{
	int ret = 0;
	ret = i2c_burst_read(charger_i2c_spec->bus, charger_i2c_spec->addr, CHARGER_CHARGING_CURRENT, charger_data->chg_charging_current, CHARGER_PROPERTY_BYTES);
	if(ret != 0)
	{
		return ret = EIO;
	}
	i2c_bytes_to_sensor_value(charger_data->chg_charging_current, val);

	return ret;
}

static int get_charging_voltage(const struct i2c_dt_spec* charger_i2c_spec, struct bq25713_data* charger_data, struct sensor_value* val)
{
	int ret = 0;
	ret = i2c_burst_read(charger_i2c_spec->bus, charger_i2c_spec->addr, CHARGER_CHARGING_VOLTAGE, charger_data->chg_charging_volt, CHARGER_PROPERTY_BYTES);
	if(ret != 0)
	{
		return ret = EIO;
	}
	i2c_bytes_to_sensor_value(charger_data->chg_charging_volt, val);

	return ret;
}

static int bq25713_init(const struct device* dev)
{
	int ret = 0;
	const struct bq25713_config* charger_config = dev->config;
    const struct i2c_dt_spec charger_i2c_spec = charger_config->i2c_spec;
	uint8_t rx_buf, tx_buf = 0;
    
    // Configure I2C controller.
	ret = i2c_charger_config(&charger_i2c_spec);
	if(ret != 0){
		return ret;
	}

	// Set charge control watchdog timer to ~5 sec.
	ret = i2c_reg_read_byte(charger_i2c_spec.bus, charger_i2c_spec.addr, CHARGE_OPTION_0, &rx_buf);
	if(ret != 0){
		return ret;
	}

	tx_buf = (rx_buf & ~BQ25713_WDT_BIT_MASK) | (BQ25713_WDT_5_SEC & BQ25713_WDT_BIT_MASK);
	ret = i2c_reg_write_byte(charger_i2c_spec.bus, charger_i2c_spec.addr, CHARGE_OPTION_0, tx_buf);
	if(ret != 0)
	{
		return ret;
	}

	return ret;
	// TODO: do some initial configuration of the charging circuit.
}

// Private extensions of the sensor API's sensor_channel enum are needed for missing sensor property types.
// When used in a switch or if statement on a value of type sensor_channel these extensions produce warnings which we suppress.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

static int bq25713_sample_fetch(const struct device* dev, enum sensor_channel chan)
{
	int ret = 0;
    const struct bq25713_config* charger_config = dev->config;
    const struct i2c_dt_spec charger_i2c_spec = charger_config->i2c_spec;
    struct bq25713_data* charger_data = dev->data;

	ret = i2c_charger_config(&charger_i2c_spec);
	if(ret != 0)
	{
		return ret;
	}

	switch(chan)
	{
		case SENSOR_CHAN_ALL:
			ret = i2c_burst_read(charger_i2c_spec.bus, charger_i2c_spec.addr, CHARGER_STATUS, charger_data->chg_status, CHARGER_PROPERTY_BYTES);
			break;

		default:
			ret = EINVAL;
			break;
	}
	return ret;
}

static int bq25713_chan_get(const struct device* dev, enum sensor_channel chan, struct sensor_value* val)
{
	int ret = 0;
    struct bq25713_data* charger_data = dev->data;

	switch(chan)
	{
		case SENSOR_CHAN_CHARGER_STATUS:
			i2c_bytes_to_sensor_value(charger_data->chg_status, val);
			break;
		default:
			ret = EINVAL;
			break;
	}

	return ret;
}

static int bq25713_attr_set(const struct device* dev, enum sensor_channel chan, enum sensor_attribute attr, const struct sensor_value* val)
{
	int ret = 0;
    const struct bq25713_config* charger_config = dev->config;
    const struct i2c_dt_spec charger_i2c_spec = charger_config->i2c_spec;

	ret = i2c_charger_config(&charger_i2c_spec);
	if(ret != 0)
	{
		return EINVAL;
	}

	switch(attr)
	{
		case SENSOR_ATTR_CONFIGURATION:
			switch (chan)
			{
				case SENSOR_CHAN_CHARGING_CONFIG:
					// First attempt to change max charging voltage.
					ret = set_charging_voltage(&charger_i2c_spec, val);
					if(ret != 0)
					{
						ret |= ERR_CHARGING_VOLTAGE_CODE;
						return ret; // Skip current if voltage configuration fails.
					}

					ret = set_charging_current(&charger_i2c_spec, &val[1]);
					if(ret != 0)
					{
						ret |= ERR_CHARGING_CURRENT_CODE;
						return ret;
					}
					break;

				case SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT:
					ret = set_charging_current(&charger_i2c_spec, val);
					break;

				case SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE:
					ret = set_charging_voltage(&charger_i2c_spec, val);
					break;
				
				default: // chan
					ret = EINVAL;
					break;
			}
			break;

		default: // attr
			ret = EINVAL;
			break;
		}
	return ret;
}

static int bq25713_attr_get(const struct device* dev, enum sensor_channel chan, enum sensor_attribute attr, struct sensor_value* val)
{
	int ret = 0;
    const struct bq25713_config* charger_config = dev->config;
    const struct i2c_dt_spec charger_i2c_spec = charger_config->i2c_spec;
	struct bq25713_data* charger_data = dev->data;

	ret = i2c_charger_config(&charger_i2c_spec);
	if(ret != 0)
	{
		return EINVAL;
	}

	switch(attr)
	{
		case SENSOR_ATTR_CONFIGURATION:
			switch (chan)
			{
				case SENSOR_CHAN_CHARGING_CONFIG:
					ret = get_charging_voltage(&charger_i2c_spec, charger_data, val);
					if(ret != 0)
					{
						ret |= ERR_CHARGING_VOLTAGE_CODE;
					}
					
					ret |= get_charging_current(&charger_i2c_spec, charger_data, &val[1]);
					if(ret != 0)
					{
						ret |= ERR_CHARGING_CURRENT_CODE;
					}
					break;

				case SENSOR_CHAN_GAUGE_DESIRED_CHARGING_CURRENT:
					ret = get_charging_current(&charger_i2c_spec, charger_data, val);
					break;

				case SENSOR_CHAN_GAUGE_DESIRED_CHARGING_VOLTAGE:
					ret = get_charging_voltage(&charger_i2c_spec, charger_data, val);
					break;
				
				default:
					ret = EINVAL;
					break;
			}
			break;

		default:
			ret = EINVAL;
		}

    return ret;
}
#pragma GCC diagnostic pop

static const struct sensor_driver_api bq25713_driver_api = 
{
	.sample_fetch = bq25713_sample_fetch,
	.channel_get = bq25713_chan_get,
	.attr_set = bq25713_attr_set,
    .attr_get = bq25713_attr_get,
};

#define BQ25713_INIT(n)						\
	static const struct bq25713_config bq25713_config_##n = {	\
        .i2c_spec = I2C_DT_SPEC_INST_GET(n),        \
	};								\
									\
	static struct bq25713_data bq25713_data_##n; \
									\
	SENSOR_DEVICE_DT_INST_DEFINE(n,					\
			    bq25713_init,				\
			    NULL,					\
			    &bq25713_data_##n,				\
			    &bq25713_config_##n,			\
			    POST_KERNEL,				\
			    CONFIG_SENSOR_INIT_PRIORITY,		\
			    &bq25713_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BQ25713_INIT);