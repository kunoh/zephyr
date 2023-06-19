#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "manager.h"
#include "message_dispatcher.h"
#include "message_handler.h"
#include "usb_hid.h"
#include "util.h"
#include "wrappers_zephyr.h"

class MessageManager : public Manager {
public:
    MessageManager(UsbHid& usb_hid, MessageProto& msg_proto, MessageDispatcher& dispatcher);
    ~MessageManager() = default;
    int Init() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    int on_battery_gen_data_cb(BatteryGeneralData sample_data);
    int on_battery_chg_data_cb(BatteryChargingData sample_data);

private:
    void HandleQueuedMessage();
    static void HandleQueuedMessageCallback(k_work* work);

private:
    UsbHid& usb_hid_;
    MessageProto& msg_proto_;
    MessageDispatcher& dispatcher_;
    char __aligned(4) msgq_buffer_[10 * sizeof(MessageBuffer)];
    k_msgq msgq_;
    k_work_wrapper<MessageManager> work_wrapper_;
    CallbackWrapper on_error_;
};