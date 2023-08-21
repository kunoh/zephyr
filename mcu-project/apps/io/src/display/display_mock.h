#pragma once

#include "display.h"
class DisplayMock : public Display {
public:
    DisplayMock();
    virtual ~DisplayMock() = default;
    void WriteRect(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height,
                   const uint8_t* rect_buf) override;
    virtual int DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                             const uint16_t height, const uint16_t pitch) override;
    virtual void NextFrame() override;
};