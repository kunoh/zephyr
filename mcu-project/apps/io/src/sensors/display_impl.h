#pragma once
#include <display.h>

class DisplayImpl : public Display {
public:
    virtual ~DisplayImpl() = default;
    bool write() override;
};