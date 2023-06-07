#include "message_manager.h"

#include <stdio.h>

#include "battery_message_encoder.h"

MessageManager::MessageManager(Logger& logger, UsbHid& usb_hid, MessageProto& msg_proto,
                               MessageDispatcher& dispatcher)
    : logger_{logger}, usb_hid_{usb_hid}, msg_proto_{msg_proto}, dispatcher_{dispatcher}
{
    k_work_init(&work_wrapper_.work, &MessageManager::HandleQueuedMessageCallback);
    work_wrapper_.self = this;
}

int MessageManager::Init()
{
    logger_.inf("Message Init");
    k_msgq_init(&msgq_, msgq_buffer_, sizeof(MessageBuffer), 10);
    usb_hid_.Init(&msgq_, &work_wrapper_.work);
    return 0;
}

void MessageManager::AddErrorCb(void (*cb)(void*), void* user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

void MessageManager::on_battery_chg_data_cb(BatteryChargingData data)
{
    MessageBuffer buffer;
    buffer.msg_type = OUTGOING;
    BatteryMessageEncoder::EncodeBatteryChargingInfo(buffer, data.des_chg_current,
                                                     data.des_chg_volt, data.status, data.charging);

    k_msgq_put(&msgq_, &buffer, K_NO_WAIT);
    k_work_submit(&work_wrapper_.work);
}

void MessageManager::on_battery_gen_data_cb(BatteryGeneralData data)
{
    MessageBuffer buffer;
    buffer.msg_type = OUTGOING;
    BatteryMessageEncoder::EncodeBatteryGeneralInfo(buffer, data.temp, data.volt, data.current,
                                                    data.remaining_capacity,
                                                    data.relative_charge_state, data.cycle_count);

    k_msgq_put(&msgq_, &buffer, K_NO_WAIT);
    k_work_submit(&work_wrapper_.work);
}

void MessageManager::HandleQueuedMessage()
{
    MessageBuffer buffer;

    while (k_msgq_get(&msgq_, &buffer, K_NO_WAIT) == 0) {
        if (buffer.msg_type == INCOMING) {
            // Decode, act, and encode response
            msg_proto_.DecodeOuterMessage(buffer);
            if (!dispatcher_.Handle(msg_proto_, buffer)) {
                logger_.wrn("Failed to Decode");
            }

            // For testing, to be deleted later
            if (0) {
                printk("Sending bytes: \n");
                size_t i;
                for (i = 0; i < buffer.length; i++) {
                    if (i > 0) printk(":");
                    printk("%02X", buffer.data[i]);
                }
                printk("\n");
            }
        }

        // Push or send response
        if (buffer.length > 0 && buffer.msg_type == OUTGOING) {
            usb_hid_.Send(buffer.data, buffer.length);
        }
    }
}

void MessageManager::HandleQueuedMessageCallback(k_work* work)
{
    k_work_wrapper<MessageManager>* wrapper =
        CONTAINER_OF(work, k_work_wrapper<MessageManager>, work);
    MessageManager* self = wrapper->self;
    self->HandleQueuedMessage();
}