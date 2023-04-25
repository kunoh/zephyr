#include "message_manager.h"

#include "battery_message_encoder.h"

MessageManager::MessageManager(Logger* logger, UsbHid* usb_hid, MessageProto* msg_proto,
                               MessageDispatcher* dispatcher, k_msgq* msgq)
    : logger_{logger},
      usb_hid_{usb_hid},
      msg_proto_{msg_proto},
      dispatcher_{dispatcher},
      msgq_{msgq}
{
    k_work_init(&work_wrapper_.work, HandleQueuedMessage);
    work_wrapper_.self = this;
}

void MessageManager::on_battery_data_cb(BatteryData data)
{
    MessageBuffer buffer;
    buffer.msg_type = OUTGOING;
    BatteryMessageEncoder::EncodeBatteryInfo(
        buffer, true, data.temp, data.volt, data.current, (int)data.remaining_capacity,
        (int)data.status, (int)data.relative_charge_state, (int)data.cycle_count);

    k_msgq_put(msgq_, &buffer, K_NO_WAIT);
    k_work_submit(&work_wrapper_.work);
}

int MessageManager::Init()
{
    logger_->inf("Message Init");
    return 0;
}

void MessageManager::AddErrorCb(void (*cb)(void*), void* user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

k_work* MessageManager::GetWorkItem()
{
    return &work_wrapper_.work;
}

void MessageManager::HandleQueuedMessage(k_work* work)
{
    k_work_wrapper<MessageManager>* wrapper =
        CONTAINER_OF(work, k_work_wrapper<MessageManager>, work);
    MessageManager* self = wrapper->self;
    MessageBuffer buffer;

    while (k_msgq_get(self->msgq_, &buffer, K_NO_WAIT) == 0) {
        if (buffer.msg_type == INCOMING) {
            // Decode, act, and encode response
            self->msg_proto_->DecodeOuterMessage(buffer);
            if (!self->dispatcher_->Handle(*self->msg_proto_, buffer)) {
                self->logger_->wrn("Failed to Decode");
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
            self->usb_hid_->Send(buffer.data, buffer.length);
        }
    }
}