#pragma once
#include <cstdint>

class Display {
public:
    virtual ~Display() = default;
    virtual void WriteRect(const uint16_t x, const uint16_t y, const uint16_t width,
                           const uint16_t height, const uint8_t* rect_buf) = 0;
    virtual int DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                             const uint16_t height, const uint16_t pitch) = 0;
    virtual void NextFrame() = 0;
};