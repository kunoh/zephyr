#include "display_mock.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(disp_mock, CONFIG_DISPLAY_LOG_LEVEL);

DisplayMock::DisplayMock()
{}

int DisplayMock::DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                              const uint16_t height, const uint16_t pitch)
{
    return 0;
}

void DisplayMock::WriteRect(const uint16_t x, const uint16_t y, const uint16_t width,
                            const uint16_t height, const uint8_t* rect_buf)
{
    return;
}

void DisplayMock::NextFrame()
{
    return;
}
