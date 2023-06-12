#pragma once

class LegControl {
public:
    virtual ~LegControl() = default;
    virtual bool Write() = 0;
    virtual bool Read(int *read_buffer) = 0;
    virtual bool Init() = 0;
};