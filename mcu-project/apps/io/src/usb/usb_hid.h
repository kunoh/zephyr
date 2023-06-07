#pragma once

#include <cstdint>

class UsbHid {
public:
    virtual ~UsbHid() = default;
    virtual int Init(void*, void*) = 0;
    virtual void Send(uint8_t* buffer, uint8_t message_length) = 0;
};