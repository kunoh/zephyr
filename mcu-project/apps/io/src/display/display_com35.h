#pragma once
#include <logger.h>
#include <display.h>

class DisplayCOM35 : public Display {
public:
    DisplayCOM35(Logger& logger);
    virtual ~DisplayCOM35() = default;
    int DisplayWrite(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height, const uint16_t pitch, const void *buf) override;
    int NextFrame() override;

private:
    Logger& logger_;
    const device* display_dev_;
};