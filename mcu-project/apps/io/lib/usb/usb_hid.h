#pragma once

#include <zephyr.h>

#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>

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

void SendReport(uint8_t *buffer, uint8_t message_length);
void SetOnIdleCallback(void (*hid_idle_cb_t)(const struct device *dev, uint16_t report_id));
void SetProtocolChangeCallback(void (*hid_protocol_cb_t)(const struct device *dev, uint8_t protocol));
void SetReportCallback(int (*hid_cb_t)(const struct device *dev,
			struct usb_setup_packet *setup, int32_t *len,
			uint8_t **data));
void StatusCb(enum usb_dc_status_code status, const uint8_t *param);
int UsbHidInit();