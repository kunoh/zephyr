/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

#include "imu_mock.h"
#include "sensor_mock.h"
#include "imu_manager.h"
#include "sensor_manager.h"

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	if (usb_enable(NULL) != 0) {
        printk("Failed to enable USB");
    }

	// PRINT
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");
	printk("Test");

	ImuMock imu_mock;
	ImuManager imu_mgr(imu_mock);

	SensorMock sen_mock;
	SensorManager sen_mgr(sen_mock);

	imu_mgr.AddSubscriber([](){printk("Got Imu Sample Data\n");});
	imu_mgr.Init();

	sen_mgr.AddSubscriber([](){printk("Got Sensor Sample Data\n");});
	sen_mgr.Init();

	while (1) {
        k_sleep(K_MSEC(1000));
	}
	return 0;
}
