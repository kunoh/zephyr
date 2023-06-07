#include "display_mock.h"

DisplayMock::DisplayMock(Logger& logger) : logger_{logger}
{}

int DisplayMock::DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                              const uint16_t height, const uint16_t pitch, const void* buf)
{
    return 0;
}

int DisplayMock::NextFrame()
{
    return 0;
}