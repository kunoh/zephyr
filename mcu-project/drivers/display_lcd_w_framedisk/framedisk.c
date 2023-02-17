/*
 * Copyright (c) 2016 Intel Corporation.
 * Copyright (c) 2021, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * framedisk is based on drivers/disk/ramdisk
 * The standard ramdisk driver uses a local static buffer s
 * This ramdisk driver gets its ramdisk_buf set to a framebuffer allocated by the display driver
 */

#include <string.h>
#include <zephyr/types.h>
#include <zephyr/drivers/disk.h>
#include <errno.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <framedisk.h>

LOG_MODULE_REGISTER(framedisk, CONFIG_DISK_LOG_LEVEL);

#define RAMDISK_SECTOR_SIZE 512
#define RAMDISK_VOLUME_SIZE  ( ( DT_PROP(DT_NODELABEL(lcdif), width) * DT_PROP(DT_NODELABEL(lcdif), height) * 3) + 512)
#define RAMDISK_SECTOR_COUNT (RAMDISK_VOLUME_SIZE / RAMDISK_SECTOR_SIZE)

static uint8_t *framedisk_buf;
void framedisk_buffer_set(uint8_t *p_buf)
{
	framedisk_buf = p_buf;
}

static void *lba_to_address(uint32_t lba)
{
	return &framedisk_buf[lba * RAMDISK_SECTOR_SIZE];
}

static int disk_ram_access_status(struct disk_info *disk)
{
	return DISK_STATUS_OK;
}

static int disk_ram_access_init(struct disk_info *disk)
{
	return 0;
}

static int disk_ram_access_read(struct disk_info *disk, uint8_t *buff,
				uint32_t sector, uint32_t count)
{
	uint32_t last_sector = sector + count;

	if (last_sector < sector || last_sector > RAMDISK_SECTOR_COUNT) {
		LOG_ERR("Sector %" PRIu32 " is outside the range %u",
			last_sector, RAMDISK_SECTOR_COUNT);
		return -EIO;
	}

	memcpy(buff, lba_to_address(sector), count * RAMDISK_SECTOR_SIZE);

	return 0;
}

static int disk_ram_access_write(struct disk_info *disk, const uint8_t *buff,
				 uint32_t sector, uint32_t count)
{
	uint32_t last_sector = sector + count;

	if (last_sector < sector || last_sector > RAMDISK_SECTOR_COUNT) {
		LOG_ERR("Sector %" PRIu32 " is outside the range %u",
			last_sector, RAMDISK_SECTOR_COUNT);
		return -EIO;
	}

	memcpy(lba_to_address(sector), buff, count * RAMDISK_SECTOR_SIZE);

	return 0;
}

static int disk_ram_access_ioctl(struct disk_info *disk, uint8_t cmd, void *buff)
{
	switch (cmd) {
	case DISK_IOCTL_CTRL_SYNC:
		break;
	case DISK_IOCTL_GET_SECTOR_COUNT:
		*(uint32_t *)buff = RAMDISK_SECTOR_COUNT;
		break;
	case DISK_IOCTL_GET_SECTOR_SIZE:
		*(uint32_t *)buff = RAMDISK_SECTOR_SIZE;
		break;
	case DISK_IOCTL_GET_ERASE_BLOCK_SZ:
		*(uint32_t *)buff  = 1U;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct disk_operations ram_disk_ops = {
	.init = disk_ram_access_init,
	.status = disk_ram_access_status,
	.read = disk_ram_access_read,
	.write = disk_ram_access_write,
	.ioctl = disk_ram_access_ioctl,
};

static struct disk_info ram_disk = {
	.name = CONFIG_MASS_STORAGE_DISK_NAME,
	.ops = &ram_disk_ops,
};

static int disk_ram_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	int rc = disk_access_register(&ram_disk);

	LOG_INF("framedisk_buf=%p, size=%d", framedisk_buf, RAMDISK_VOLUME_SIZE);
	return rc;
}

SYS_INIT(disk_ram_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
