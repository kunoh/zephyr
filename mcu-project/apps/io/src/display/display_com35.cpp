#include "display_com35.h"

#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#include "display_lcd.h"

DisplayCOM35::DisplayCOM35(Logger& logger) : logger_{logger}
{
    display_dev_ = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev_)) {
        logger_.err("Display not found. Aborting...");
    }
}

int DisplayCOM35::DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                               const uint16_t height, const uint16_t pitch, const void* buf)
{
    display_buffer_descriptor buf_desc = {.width = width, .height = height, .pitch = pitch};
    return display_write(display_dev_, x, y, &buf_desc, buf);
}

int DisplayCOM35::NextFrame()
{
    display_lcd_next_framebuffer_ready(display_dev_);
    return 0;
}