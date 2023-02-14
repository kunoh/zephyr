#pragma once

class Display {
public:
    virtual ~Display() = default;
    virtual bool write() = 0;
};