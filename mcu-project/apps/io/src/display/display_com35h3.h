#pragma once
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include "display.h"

class DisplayCom35h3 : public Display {
public:
    DisplayCom35h3();
    virtual ~DisplayCom35h3() = default;
    int DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width,
                     const uint16_t height, const uint16_t pitch, const void* buf) override;
    int NextFrame() override;

private:
    const device* display_dev_;
};