#pragma once

// stddef and stdint is needed by usb/class/usb_hid.h, but not included
// therefore, they are included here
#include <stddef.h>
#include <stdint.h>
//
#include <zephyr/usb/class/usb_hid.h>

#include "logger.h"
#include "usb_hid.h"
#include "wrappers_zephyr.h"

class UsbHidZephyr : public UsbHid {
public:
    UsbHidZephyr(Logger &logger);
    virtual ~UsbHidZephyr() = default;
    int Init(void *message_queue, void *work_queue) override;
    void Send(uint8_t *buffer, uint8_t message_length) override;

private:
    int HandleReceivedMessage(int32_t *len, uint8_t **data);
    static int HandleReceiveMessageCallback(const device *dev, usb_setup_packet *setup,
                                            int32_t *len, uint8_t **data);
    static void HandleProtocolChange(const struct device *dev, uint8_t protocol);
    static void HandleOnIdle(const struct device *dev, uint16_t report_id);
    static void HandleIntInReady(const struct device *dev);

private:
    Logger &logger_;

    k_msgq *rx_msgq_;
    k_work *rx_work_;
    self_wrapper<UsbHidZephyr, device> hdev_;

    hid_ops ops_;
};