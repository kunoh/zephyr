#pragma once

class Inclinometer {
public:
    virtual ~Inclinometer() = default;
    virtual bool write() = 0;
};