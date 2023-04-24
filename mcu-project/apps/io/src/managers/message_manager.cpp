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
    k_work_init(&work_, HandleQueuedMessage);
}

void MessageManager::on_battery_data_cb(BatteryData data)
{
    MessageBuffer buffer;
    buffer.msg_type = OUTGOING;
    BatteryMessageEncoder::EncodeBatteryInfo(
        buffer, true, data.temp, data.volt, data.current, (int)data.remaining_capacity,
        (int)data.status, (int)data.relative_charge_state, (int)data.cycle_count);

    k_msgq_put(msgq_, &buffer, K_NO_WAIT);
    k_work_submit(&work_);
}

k_work* MessageManager::GetWorkItem()
{
    return &work_;
}

void MessageManager::HandleQueuedMessage(k_work* work)
{
    MessageManager* self = CONTAINER_OF(work, MessageManager, work_);
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