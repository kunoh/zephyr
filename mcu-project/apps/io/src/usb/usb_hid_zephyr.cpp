#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>

#include "usb_hid_zephyr.h"

static K_SEM_DEFINE(usb_hid_sem_, 0, 1);

static const uint8_t hid_report_desc[] = {
    HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
    HID_USAGE(HID_USAGE_GEN_DESKTOP_UNDEFINED),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_LOGICAL_MIN8(0x00),
        HID_LOGICAL_MAX16(0xFF, 0x00),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(1),
        HID_INPUT(0x02),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(64),
        HID_INPUT(0x02),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(1),
        HID_OUTPUT(0x02),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(64),
        HID_OUTPUT(0x02),
    HID_END_COLLECTION,
};

UsbHidZephyr::UsbHidZephyr(Logger& logger)
    : logger_{logger}
{
    hdev_ = device_get_binding("HID_0");
    if (hdev_ == NULL) {
        logger_.err("Cannot get USB HID Device");
        init_status_ = -ENODEV;
        return;
    }

    ops_.get_report = NULL;
    ops_.set_report = HandleSetReport;
    ops_.protocol_change = HandleProtocolChange;
    ops_.on_idle = HandleOnIdle;
    ops_.int_in_ready = HandleIntInReady;

    usb_hid_register_device(hdev_, hid_report_desc, sizeof(hid_report_desc),
                &ops_);

    init_status_ = usb_hid_set_proto_code(hdev_, HID_BOOT_IFACE_CODE_NONE);
    if (init_status_ != 0) {
        logger_.err("Failed to set Protocol Code");
        return;
    }

    init_status_ = usb_hid_init(hdev_);
    if (init_status_ != 0) {
        logger_.err("Failed to initialize HID device");
        return;
    }
}

void UsbHidZephyr::Send(uint8_t *buffer, uint8_t message_length)
{
    int ret;
    uint32_t wrote;

    k_sem_take(&usb_hid_sem_, K_MSEC(30));
    ret = hid_int_ep_write(hdev_, buffer, message_length, &wrote);
    if (ret != 0) {
        /*
            * Do nothing and wait until host has reset the device
            * and hid_ep_in_busy is cleared.
            */
        logger_.err("Failed to submit report");
        k_sem_give(&usb_hid_sem_);
    } else {
        logger_.dbg("Report submitted");
    }
}

void UsbHidZephyr::SetReceiveCallback(int (*hid_cb_t)(const struct device *dev,
                                                struct usb_setup_packet *setup, int32_t *len,
                                                uint8_t **data))
{
    ops_.set_report = hid_cb_t;
}

int UsbHidZephyr::HandleSetReport(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    printk("Please set 'set_report' callback\n");
    return 0;
}

void UsbHidZephyr::HandleProtocolChange(const struct device *dev, uint8_t protocol)
{
    printk("New protocol: %s\n", protocol == HID_PROTOCOL_BOOT ?
        "boot" : "report");
}

void UsbHidZephyr::HandleOnIdle(const struct device *dev, uint16_t report_id)
{
    printk("On idle callback\n");
}

void UsbHidZephyr::HandleIntInReady(const struct device *dev)
{
    k_sem_give(&usb_hid_sem_);
}
