#pragma once
#include <cstdint>

class LegControl {
public:
    virtual ~LegControl() = default;
    virtual bool Write() = 0;
    virtual bool Read(int *ReadBuffer) = 0;
    virtual bool Init() = 0;
};