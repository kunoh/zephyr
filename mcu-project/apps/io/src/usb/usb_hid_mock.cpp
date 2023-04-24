#include "usb_hid_mock.h"

#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

UsbHidMock::UsbHidMock(Logger &logger) : logger_{logger}
{}

void UsbHidMock::Send(uint8_t *buffer, uint8_t message_length)
{}

void UsbHidMock::SetReceiveCallback(int (*hid_cb_t)(const struct device *dev,
                                                    struct usb_setup_packet *setup, int32_t *len,
                                                    uint8_t **data))
{}