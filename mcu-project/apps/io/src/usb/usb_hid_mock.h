#pragma once

// stddef and stdint is needed by usb/class/usb_hid.h, but not included
// therefore, they are included here
#include <stddef.h>
#include <stdint.h>
#include <zephyr/usb/class/usb_hid.h>

#include "logger.h"
#include "usb_hid.h"

class UsbHidMock : public UsbHid {
public:
    UsbHidMock(Logger &logger);
    virtual ~UsbHidMock() = default;
    void Send(uint8_t *buffer, uint8_t message_length) override;

    void SetReceiveCallback(int (*hid_cb_t)(const struct device *dev,
                                            struct usb_setup_packet *setup, int32_t *len,
                                            uint8_t **data));

private:
    Logger &logger_;
};