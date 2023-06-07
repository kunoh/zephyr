#include "usb_hid_zephyr.h"

#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>

#include "message_handler.h"

K_SEM_DEFINE(usb_hid_sem_, 0, 1);

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

UsbHidZephyr::UsbHidZephyr(Logger &logger) : logger_{logger}
{}

int UsbHidZephyr::Init(void *message_queue, void *work_queue)
{
    int ret = 0;
    const device *hdev = device_get_binding("HID_0");
    if (hdev == NULL) {
        logger_.err("Cannot get USB HID Device");
        return -ENODEV;
    }
    hdev_.p = *(hdev);

    ops_.get_report = NULL;
    ops_.set_report = &UsbHidZephyr::HandleReceiveMessageCallback;
    ops_.protocol_change = &UsbHidZephyr::HandleProtocolChange;
    ops_.on_idle = &UsbHidZephyr::HandleOnIdle;
    ops_.int_in_ready = &UsbHidZephyr::HandleIntInReady;

    usb_hid_register_device(&hdev_.p, hid_report_desc, sizeof(hid_report_desc), &ops_);

    ret = usb_hid_set_proto_code(&hdev_.p, HID_BOOT_IFACE_CODE_NONE);
    if (ret != 0) {
        logger_.err("Failed to set Protocol Code");
        return ret;
    }

    ret = usb_hid_init(&hdev_.p);
    if (ret != 0) {
        logger_.err("Failed to initialize HID device");
        return ret;
    }

    hdev_.self = this;
    rx_msgq_ = reinterpret_cast<k_msgq *>(message_queue);
    rx_work_ = reinterpret_cast<k_work *>(work_queue);
    k_work_submit(rx_work_);
    return 0;
}

void UsbHidZephyr::Send(uint8_t *buffer, uint8_t message_length)
{
    int ret;
    uint32_t wrote;

    k_sem_take(&usb_hid_sem_, K_MSEC(30));
    ret = hid_int_ep_write(&hdev_.p, buffer, message_length, &wrote);
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

int UsbHidZephyr::HandleReceivedMessage(int32_t *len, uint8_t **data)
{
    MessageBuffer buffer;
    memcpy(buffer.data, *data, (size_t)*len);
    buffer.length = *len;
    buffer.msg_type = INCOMING;

    // For testing, to be deleted later
    if (0) {
        printk("Received bytes: \n");
        size_t i;
        for (i = 0; i < buffer.length; i++) {
            if (i > 0) printk(":");
            printk("%02X", buffer.data[i]);
        }
        printk("\n");
    }
    //

    while (k_msgq_put(rx_msgq_, &buffer, K_NO_WAIT) != 0) {
        // message queue is full: purge old data & try again
        k_msgq_purge(rx_msgq_);
    }

    return k_work_submit(rx_work_);
}

int UsbHidZephyr::HandleReceiveMessageCallback(const device *dev, usb_setup_packet *setup,
                                               int32_t *len, uint8_t **data)
{
    typedef self_wrapper<UsbHidZephyr, device> wrapper_template;
    wrapper_template *wrapper = CONTAINER_OF(dev, wrapper_template, p);
    UsbHidZephyr *self = wrapper->self;
    return self->HandleReceivedMessage(len, data);
}

void UsbHidZephyr::HandleProtocolChange(const struct device *dev, uint8_t protocol)
{
    printk("New protocol: %s\n", protocol == HID_PROTOCOL_BOOT ? "boot" : "report");
}

void UsbHidZephyr::HandleOnIdle(const struct device *dev, uint16_t report_id)
{
    printk("On idle callback\n");
}

void UsbHidZephyr::HandleIntInReady(const struct device *dev)
{
    k_sem_give(&usb_hid_sem_);
}