#pragma once
#include "leg_control.h"

class LegControlImpl : public LegControl {
public:
    virtual ~LegControlImpl() = default;
    bool Write() override;
    bool Read(int *read_buffer) override;
    bool Init() override;
};