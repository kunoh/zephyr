#include "message_thread.h"
#include "message_dispatcher.h"
#include "lib/usb/usb_hid.h"
#include "system_message_handler_impl.h"
#include "display_message_handler_impl.h"
#include "inclinometer_message_handler_impl.h"
#include "logger_impl.h"
#include "display_impl.h"
#include "inclinometer_impl.h"

LOG_MODULE_REGISTER(message_thread, LOG_LEVEL_INF);

K_MSGQ_DEFINE(usb_hid_msgq, sizeof(MessageBuffer), 10, 1);

static int SetReportCbCustom(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
	LOG_DBG("Set report callback");
    MessageBuffer buffer;
    memcpy(buffer.data, *data, (size_t)*len);
    buffer.length = *len;
    buffer.is_received = true;

    // For testing, to be deleted later
    if (0){
        printk("Received bytes: \n");
        size_t i;
        for (i = 0; i < buffer.length; i++)
        {
            if (i > 0) printk(":");
            printk("%02X", buffer.data[i]);
        }
        printk("\n");
    }
    //

    while (k_msgq_put(&usb_hid_msgq, &buffer, K_NO_WAIT) != 0) {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&usb_hid_msgq);
    }
	return 0;
}

void MessageThreadRun(void)
{
    MessageProto msg_proto;
    MessageBuffer buffer;
    MessageDispatcher dispatcher;

    DisplayImpl disp;
    InclinometerImpl incl;
    LoggerImpl logger("");

    SystemMessageHandlerImpl sys_impl(logger);
    DisplayMessageHandlerImpl disp_impl(logger, disp);
    InclinometerMessageHandlerImpl incl_impl(logger, incl);

    dispatcher.AddHandler(sys_impl);
    dispatcher.AddHandler(disp_impl);
    dispatcher.AddHandler(incl_impl);

    SetReportCallback(SetReportCbCustom);
    UsbHidInit();

	if (usb_enable(StatusCb) != 0) {
		LOG_ERR("Failed to enable USB");
		//return;
	}

    while (1) {
        k_msgq_get(&usb_hid_msgq, &buffer, K_FOREVER);

        if (buffer.is_received){
            msg_proto.DecodeOuterMessage(buffer);
            if (!dispatcher.Handle(msg_proto, buffer)) {
                LOG_WRN("Failed to Decode");
            }
        }

        // For testing, to be deleted later
        if (0){
            printk("Sending bytes: \n");
            size_t i;
            for (i = 0; i < buffer.length; i++)
            {
                if (i > 0) printk(":");
                printk("%02X", buffer.data[i]);
            }
            printk("\n");
        }
        //

        if (buffer.length > 0) {
        	LOG_DBG("Sending response");
            SendReport(buffer.data, buffer.length);
        }
    }
}