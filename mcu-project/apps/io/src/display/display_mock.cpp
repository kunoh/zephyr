#include "display_mock.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(disp_mock, CONFIG_DISPLAY_LOG_LEVEL);

DisplayMock::DisplayMock()
{}

int DisplayMock::DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                              const uint16_t height, const uint16_t pitch, const void* buf)
{
    return 0;
}

int DisplayMock::NextFrame()
{
    LOG_INF("Next Frame!");
    return 0;
}