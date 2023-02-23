#pragma once

#include "message_handler.h"
#include "message_dispatcher.h"

class MessageManager {
public:
    MessageManager(Logger* logger, UsbHid* usb_hid, MessageProto* msg_proto, MessageDispatcher* dispatcher, k_msgq* msgq);
    ~MessageManager() = default;
    k_work* GetWorkItem();

private:
    static void HandleReceivedMessage(k_work *work);

private:
    Logger* logger_;
    UsbHid* usb_hid_;
    MessageProto* msg_proto_;
    MessageDispatcher* dispatcher_;
    k_msgq* msgq_;
    k_work work_;
};