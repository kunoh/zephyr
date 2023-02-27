#pragma once
#include <cstdint>

class Display {
public:
    virtual ~Display() = default;
    virtual int DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                             const uint16_t height, const uint16_t pitch, const void *buf) = 0;
    virtual int NextFrame() = 0;
};