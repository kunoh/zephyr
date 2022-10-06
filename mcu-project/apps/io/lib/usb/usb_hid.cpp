
#include "usb_hid.h"

LOG_MODULE_REGISTER(usb_hid_tm, LOG_LEVEL_INF);

static bool configured;
static const struct device *hdev;
static K_SEM_DEFINE(hid_sem, 1, 1);

void SendReport(uint8_t *buffer, uint8_t message_length)
{
	int ret; 
	uint32_t wrote;

	k_sem_take(&hid_sem, K_MSEC(30));
	ret = hid_int_ep_write(hdev, buffer, message_length, &wrote);
	if (ret != 0) {
		/*
			* Do nothing and wait until host has reset the device
			* and hid_ep_in_busy is cleared.
			*/
		LOG_ERR("Failed to submit report");
		k_sem_give(&hid_sem);
	} else {
		LOG_DBG("Report submitted");
	}
}

static void IntInReadyCb(const struct device *dev) 
{
	k_sem_give(&hid_sem); 
}

static int SetReportCb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
	LOG_WRN("Please set 'set_report' callback");
	return 0;
}

static void ProtocolCb(const struct device *dev, uint8_t protocol)
{
	LOG_INF("New protocol: %s", protocol == HID_PROTOCOL_BOOT ?
		"boot" : "report");
}

static void OnIdleCb(const struct device *dev, uint16_t report_id)
{
	LOG_DBG("On idle callback");
}

static struct hid_ops ops = {
	NULL,
	SetReportCb,
	ProtocolCb,
	OnIdleCb,
	IntInReadyCb
};

void SetReportCallback(int (*hid_cb_t)(const struct device *dev,
			struct usb_setup_packet *setup, int32_t *len,
			uint8_t **data))
{
	ops.set_report = hid_cb_t;
}

void SetProtocolChangeCallback(void (*hid_protocol_cb_t)(const struct device *dev, uint8_t protocol))
{
	ops.protocol_change = hid_protocol_cb_t;
}

void SetOnIdleCallback(void (*hid_idle_cb_t)(const struct device *dev, uint16_t report_id))
{
	ops.on_idle = hid_idle_cb_t;
}

void StatusCb(enum usb_dc_status_code status, const uint8_t *param)
{
	switch (status) {
	case USB_DC_RESET:
		configured = false;
		break;
	case USB_DC_CONFIGURED:
		if (!configured) {
			IntInReadyCb(hdev);
			configured = true;
		}
		break;
	case USB_DC_SOF:
		break;
	default:
		LOG_DBG("status %u unhandled", status);
		break;
	}
}

int UsbHidInit(){
    int ret;

    hdev = device_get_binding("HID_0");
	if (hdev == NULL) {
		LOG_ERR("Cannot get USB HID Device");
		return -ENODEV;
	}

	LOG_INF("HID Device: dev %p", hdev);

	usb_hid_register_device(hdev, hid_report_desc, sizeof(hid_report_desc),
				&ops);

    ret = usb_hid_set_proto_code(hdev, HID_BOOT_IFACE_CODE_NONE);
	if (ret != 0) {
		LOG_WRN("Failed to set Protocol Code");
        return ret;
	}

    ret = usb_hid_init(hdev);
    if (ret != 0) {
        LOG_ERR("Failed to initialize HID device");
        return ret;
    }

	return 0;
}