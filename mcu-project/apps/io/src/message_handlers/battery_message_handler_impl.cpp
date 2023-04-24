#include "battery_message_handler_impl.h"

#include "stdio.h"  // For testing
#include "util.h"

BatteryMessageHandlerImpl::BatteryMessageHandlerImpl(Logger& logger,
                                                     BatteryManager& battery_manager,
                                                     MessageManager& msg_manager)
    : logger_{logger}, battery_manager_{battery_manager}, msg_manager_{msg_manager}
{}

bool BatteryMessageHandlerImpl::HandleBatteryInfo(MessageProto& msg, MessageBuffer& buffer)
{
    BatteryInfo bi = BatteryInfo_init_zero;
    if (!msg.DecodeInnerMessage(BatteryInfo_fields, &bi)) {
        return false;
    }
    return true;
}

/// @brief Have the battery manager trigger a battery sampling, get the sample
///        and encode it as a BatteryInfo response to the CPU.
bool BatteryMessageHandlerImpl::HandleRequestBatteryInfo(MessageProto& msg, MessageBuffer& buffer)
{
    int ret = 0;
    bool valid = true;
    RequestBatteryInfo bi = RequestBatteryInfo_init_zero;
    if (!msg.DecodeInnerMessage(RequestBatteryInfo_fields, &bi)) {
        return false;
    }

    BatteryData data;
    ret = battery_manager_.TriggerAndGetSample(&data);
    if (ret != 0) {
        logger_.wrn(
            "Failed to trigger and get sample by polling. Responding with empty BatteryInfo.");
        valid = false;
    }

    // Encode response
    ret = BatteryMessageEncoder::EncodeBatteryInfo(
        buffer, valid, data.temp, data.volt, data.current, (int)data.remaining_capacity,
        (int)data.status, (int)data.relative_charge_state, (int)data.cycle_count);
    if (!ret) {
        logger_.wrn("Failed to encode BatteryInfo.");
        return false;
    }
    buffer.msg_type = OUTGOING;

    return true;
}

/// @brief Add CPU as receiver of periodic battery information and encode a response.
bool BatteryMessageHandlerImpl::HandleRequestBatteryNotifications(MessageProto& msg,
                                                                  MessageBuffer& buffer)
{
    RequestBatteryNotifications bn = RequestBatteryNotifications_init_zero;
    if (!msg.DecodeInnerMessage(RequestBatteryNotifications_fields, &bn)) {
        return false;
    }

    if (bn.enable) {
        if (!battery_manager_.GetCpuSubscribed()) {
            battery_manager_.AddSubscriber(
                [&](BatteryData sample_data) { msg_manager_.on_battery_data_cb(sample_data); });
            battery_manager_.SetCpuSubscribed(true);
        }

        // Encode response
        if (!BatteryMessageEncoder::EncodeResponseBatteryNotifications(buffer)) {
            logger_.wrn("Failed to Encode ResponseBatteryNotifications");
            return false;
        }
        buffer.msg_type = OUTGOING;
    }

    return true;
}

// Read status field from ResponseBatteryNotifications message
bool BatteryMessageHandlerImpl::HandleResponseBatteryNotifications(MessageProto& msg,
                                                                   MessageBuffer& buffer)
{
    ResponseBatteryNotifications bn = ResponseBatteryNotifications_init_zero;
    if (!msg.DecodeInnerMessage(ResponseBatteryNotifications_fields, &bn)) {
        return false;
    }

    return true;
}
