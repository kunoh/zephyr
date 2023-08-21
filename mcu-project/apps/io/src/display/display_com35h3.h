#pragma once
#include <zephyr/device.h>

#include "display.h"

class DisplayCom35h3 : public Display {
public:
    DisplayCom35h3();
    virtual ~DisplayCom35h3() = default;
    void WriteRect(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height,
                   const uint8_t* rect_buf);
    int DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                     const uint16_t height, const uint16_t pitch) override;
    void NextFrame() override;

private:
    const device* display_dev_;
    uint8_t* framebuf_[2];
    uint8_t framebuf_idx_;
};