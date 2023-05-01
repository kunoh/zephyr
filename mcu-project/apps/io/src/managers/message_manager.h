#pragma once

#include <zephyr/kernel.h>

#include "logger.h"
#include "manager.h"
#include "message_dispatcher.h"
#include "message_handler.h"
#include "usb_hid.h"
#include "util.h"
#include "wrappers_zephyr.h"

class MessageManager : public Manager {
public:
    MessageManager(Logger* logger, UsbHid* usb_hid, MessageProto* msg_proto,
                   MessageDispatcher* dispatcher, k_msgq* msgq);
    ~MessageManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    k_work* GetWorkItem();
    void on_battery_gen_data_cb(BatteryGeneralData sample_data);
    void on_battery_chg_data_cb(BatteryChargingData sample_data);

private:
    static void HandleQueuedMessage(k_work* work);

private:
    static int error;
    Logger* logger_;
    UsbHid* usb_hid_;
    MessageProto* msg_proto_;
    MessageDispatcher* dispatcher_;
    k_msgq* msgq_;
    k_work_wrapper<MessageManager> work_wrapper_;
    CbWrapper on_error_{.user_data = NULL, .cb = NULL};
};