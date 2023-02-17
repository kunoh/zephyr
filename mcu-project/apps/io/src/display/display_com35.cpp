#include <zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <display_lcd.h>
#include "display_com35.h"

DisplayCOM35::DisplayCOM35(Logger& logger)
    : logger_{logger}
{
    display_dev_ = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev_)) {
        //logger_.err("Device %s not found. Aborting...", display_dev_->name);
        logger_.err("Device not found. Aborting...");
    }

}

int DisplayCOM35::DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height, const uint16_t pitch, const void *buf)
{
    display_buffer_descriptor buf_desc = { .width = width, .height = height, .pitch = pitch };
    return display_write(display_dev_, x, y, &buf_desc, buf);
}

int DisplayCOM35::NextFrame()
{
    display_lcd_next_framebuffer_ready(display_dev_);
    return 0;
}