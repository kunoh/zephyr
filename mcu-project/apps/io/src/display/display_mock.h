#pragma once

#include "display.h"
class DisplayMock : public Display {
public:
    DisplayMock();
    virtual ~DisplayMock() = default;
    int DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                     const uint16_t height, const uint16_t pitch, const void* buf) override;
    int NextFrame() override;

private:
};