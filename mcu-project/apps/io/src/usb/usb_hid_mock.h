#pragma once

// stddef and stdint is needed by usb/class/usb_hid.h, but not included
// therefore, they are included here
#include <stddef.h>
#include <stdint.h>

#include "usb_hid.h"

class UsbHidMock : public UsbHid {
public:
    UsbHidMock();
    virtual ~UsbHidMock() = default;
    int Init(void *message_queue, void *work_queue) override;
    void Send(uint8_t *buffer, uint8_t message_length) override;

private:
};