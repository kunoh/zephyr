#include "message_thread.h"

LOG_MODULE_REGISTER(message_thread, LOG_LEVEL_INF);

K_MSGQ_DEFINE(usb_hid_msgq, sizeof(MessageBuffer), 10, 1);

static int SetReportCbCustom(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
	LOG_DBG("Set report callback");
    MessageBuffer message;
    memcpy(message.buffer, *data, (size_t)*len);
    message.message_length = *len;
    message.is_received = true;
    while (k_msgq_put(&usb_hid_msgq, &message, K_NO_WAIT) != 0) {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&usb_hid_msgq);
    }
	return 0;
}

void MessageThreadRun(void) {

    Message msg;
    MessageBuffer message;
    MessageDispatcher dispatcher;
    dispatcher.Init();

    SetReportCallback(SetReportCbCustom);
    UsbHidInit();

	if (usb_enable(StatusCb) != 0) {
		LOG_ERR("Failed to enable USB");
		return;
	}

    while (1) {
        k_msgq_get(&usb_hid_msgq, &message, K_FOREVER);

        if (message.is_received){
            msg.InitializeMessageOuter(message.buffer, message.message_length);
            if (!dispatcher.Handle(message, msg)) {
                LOG_WRN("Failed to Decode");
            }
        }

        if (message.message_length > 0) {
        	LOG_DBG("Sending response");
            SendReport(message.buffer, message.message_length);
        }
    }
}