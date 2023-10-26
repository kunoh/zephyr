/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>

#include "imu_mock.h"
#include "sensor_mock.h"
#include "imu_manager.h"
#include "sensor_manager.h"

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
	.type = FS_LITTLEFS,
	.mnt_point = "/lfs",
	.fs_data = &storage,
	.storage_dev = (void *)FIXED_PARTITION_ID(storage_partition),
};

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	if (usb_enable(NULL) != 0) {
        printk("Failed to enable USB");
    }

    int rc;
    rc = fs_mount(&lfs_storage_mnt);
    if (rc < 0) {
        printk("Error mounting littlefs [%d]", rc);
    }

	// PRINT
	
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
