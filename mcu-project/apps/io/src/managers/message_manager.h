#pragma once

#include <zephyr/kernel.h>

#include "logger.h"
#include "message_dispatcher.h"
#include "message_handler.h"
#include "usb_hid.h"
#include "util.h"

class MessageManager {
public:
    MessageManager(Logger* logger, UsbHid* usb_hid, MessageProto* msg_proto,
                   MessageDispatcher* dispatcher, k_msgq* msgq);
    ~MessageManager() = default;
    k_work* GetWorkItem();
    void on_battery_data_cb(BatteryData data);

private:
    static void HandleQueuedMessage(k_work* work);

private:
    Logger* logger_;
    UsbHid* usb_hid_;
    MessageProto* msg_proto_;
    MessageDispatcher* dispatcher_;
    k_msgq* msgq_;
    k_work work_;
};