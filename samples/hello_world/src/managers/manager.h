#pragma once

class Manager {
public:
    virtual ~Manager() = default;
    virtual int Init() = 0;
    virtual int Selftest() = 0;
};