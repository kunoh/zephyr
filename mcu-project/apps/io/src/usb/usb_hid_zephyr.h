#pragma once

// stddef and stdint is needed by usb/class/usb_hid.h, but not included
// therefore, they are included here
#include <stddef.h>
#include <stdint.h>
//
#include <zephyr/usb/class/usb_hid.h>

#include "logger.h"
#include "usb_hid.h"

class UsbHidZephyr : public UsbHid {
public:
    UsbHidZephyr(Logger &logger);
    virtual ~UsbHidZephyr() = default;
    void Send(uint8_t *buffer, uint8_t message_length) override;
    void SetReceiveCallback(int (*hid_cb_t)(const struct device *dev,
                                            struct usb_setup_packet *setup, int32_t *len,
                                            uint8_t **data));

private:
    static int HandleSetReport(const struct device *dev, struct usb_setup_packet *setup,
                               int32_t *len, uint8_t **data);
    static void HandleProtocolChange(const struct device *dev, uint8_t protocol);
    static void HandleOnIdle(const struct device *dev, uint16_t report_id);
    static void HandleIntInReady(const struct device *dev);

private:
    Logger &logger_;
    int init_status_ = 0;
    const device *hdev_;
    hid_ops ops_;
};