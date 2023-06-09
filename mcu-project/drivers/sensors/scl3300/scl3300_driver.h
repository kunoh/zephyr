#pragma once
#include <stdio.h>

#include <zephyr/drivers/spi.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

/// Inclinometer driver status codes
#define INCL_STATUS_OK					0		///	General OK
#define INCL_ERR_INVALID_ARGUMENT		1		/// Invalid argument passed to function.
#define INCL_ERR_INCORRECT_CRC			2		/// Incorrect CRC value return from device.
#define INCL_ERR_DEVICE_REPORTS_ERR		3		/// Inclinometer reports error.
#define INCL_ERR_SPI_ERR				4		/// General SPI error.
#define INCL_ERR_SPI_NOT_RDY			5		/// SPI not ready for use.

#define INCL_ERR_DEVICE_IN_PWR_DOWN		8		/// Inclinometer is in power down mode.

enum InclMode {
	// Attributes the user can set/get
	SCL3300_CONFIG_MODE_1 = 1,
	SCL3300_CONFIG_MODE_2,
	SCL3300_CONFIG_MODE_3,
	SCL3300_CONFIG_MODE_4,
	SCL3300_CONFIG_MODE_PWR_DOWN
};

struct scl3300_config {
	struct spi_dt_spec spi;
	char *spi_name;
	uint8_t spi_chipselect;
};

struct scl3300_data {
	struct k_sem sem;
	
	uint8_t incl_operating_mode;

	int16_t AccX_mms;	
	int16_t AccY_mms;		
	int16_t AccZ_mms;
	

	struct sensor_value accX_mms;		/// Acceleration in X dirction in meters/second²
	struct sensor_value accY_mms;		/// Acceleration in Y dirction in meters/second²
	struct sensor_value accZ_mms;		/// Acceleration in Z dirction in meters/second²

	struct sensor_value angleX_cdeg;	/// Angle in degrees.
	struct sensor_value angleY_cdeg;	/// Angle in degrees.
	struct sensor_value angleZ_cdeg;	/// Angle in degrees.
};

/** @brief Sensor specific channels of scl3300. */
enum scl3300_channel{
    /// @brief Inclinometer angle outputs.
    SENSOR_CHAN_ANGLE_X = SENSOR_CHAN_PRIV_START,
    SENSOR_CHAN_ANGLE_Y,
	SENSOR_CHAN_ANGLE_Z,
	SENSOR_CHAN_ANGLE_XYZ,
};
