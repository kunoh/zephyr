#include "display_com35h3.h"

#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/disk_access.h>

LOG_MODULE_REGISTER(disp_com35h3, CONFIG_DISPLAY_LOG_LEVEL);

#define WIDTH DT_PROP(DT_NODELABEL(lcdif), width)
#define HEIGHT DT_PROP(DT_NODELABEL(lcdif), height)
#define PIXEL_BYTES 3
#define FRAMEBUF_SIZE WIDTH *HEIGHT *PIXEL_BYTES

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
K_HEAP_DEFINE(framebuf_pool, 2 * (FRAMEBUF_SIZE + 512));

static const char *disk_pdrv = "RAM";

void framedisk_buffer_set(uint8_t *p_buf)
{
    uint32_t cmd_buf = (uint32_t)p_buf;
    LOG_INF("Framedisk addr = 0x%x", cmd_buf);
    if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_SET_RAMBUF_ADDR, &cmd_buf) != 0) {
        LOG_ERR("Disk ioctl DISK_IOCTL_SET_RAMBUF_ADDR failed");
    }
    return;
}

DisplayCom35h3::DisplayCom35h3() : framebuf_idx_{0}
{
    display_dev_ = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev_)) {
        LOG_ERR("Display not found. Aborting...");
    }

    if (disk_access_init(disk_pdrv) != 0) {
        LOG_ERR("disk_access_init(%s)", disk_pdrv);
    }

    for (unsigned i = 0; i < sizeof(framebuf_) / sizeof(uint8_t *); i++) {
        framebuf_[i] = (uint8_t *)k_heap_alloc(&framebuf_pool, FRAMEBUF_SIZE, K_NO_WAIT);
        if (framebuf_[i] == NULL) {
            LOG_ERR("Could not allocate frame buffer %d (0x%x)", i, FRAMEBUF_SIZE);
        } else {
            LOG_INF("framebuf_[%d] = %p", i, (void *)framebuf_[i]);
        }
        memset(framebuf_[i], 0x00, FRAMEBUF_SIZE);
    }
    framedisk_buffer_set(framebuf_[framebuf_idx_]);
}

void DisplayCom35h3::WriteRect(const uint16_t x, const uint16_t y, const uint16_t width,
                               const uint16_t height, const uint8_t *rect_buf)
{
    uint8_t write_idx = framebuf_idx_;
    const uint8_t *src = rect_buf;
    uint8_t *dst = framebuf_[write_idx];

    dst += PIXEL_BYTES * (y * WIDTH + x);
    for (int h_idx = 0; h_idx < height; h_idx++) {
        memcpy(dst, src, PIXEL_BYTES * width);
        src += PIXEL_BYTES * width;
        dst += PIXEL_BYTES * WIDTH;
    }
}

int DisplayCom35h3::DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                                 const uint16_t height, const uint16_t pitch)
{
    display_buffer_descriptor buf_desc = {.buf_size = (uint32_t)(width * height * pitch),
                                          .width = width,
                                          .height = height,
                                          .pitch = pitch};
    return display_write(display_dev_, x, y, &buf_desc, framebuf_[framebuf_idx_]);
}

void DisplayCom35h3::NextFrame()
{
    framebuf_idx_ = !framebuf_idx_;
    framedisk_buffer_set(framebuf_[framebuf_idx_]);
}
