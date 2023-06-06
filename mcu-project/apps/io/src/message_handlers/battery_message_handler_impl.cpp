#include "battery_message_handler_impl.h"

#include "stdio.h"  // For testing
#include "util.h"

BatteryMessageHandlerImpl::BatteryMessageHandlerImpl(Logger& logger,
                                                     BatteryManager& battery_manager,
                                                     MessageManager& msg_manager)
    : logger_{logger}, battery_manager_{battery_manager}, msg_manager_{msg_manager}
{}

bool BatteryMessageHandlerImpl::HandleBatteryGeneralInfo(MessageProto& msg, MessageBuffer& buffer)
{
    BatteryGeneralInfo bgi = BatteryGeneralInfo_init_zero;
    if (!msg.DecodeInnerMessage(BatteryGeneralInfo_fields, &bgi)) {
        return false;
    }
    return true;
}

bool BatteryMessageHandlerImpl::HandleBatteryChargingInfo(MessageProto& msg, MessageBuffer& buffer)
{
    BatteryChargingInfo bci = BatteryChargingInfo_init_zero;
    if (!msg.DecodeInnerMessage(BatteryChargingInfo_fields, &bci)) {
        return false;
    }
    return true;
}

/// @brief Have the battery manager get the last general battery sample
///        and encode it as a BatteryGeneralInfo response to the CPU.
bool BatteryMessageHandlerImpl::HandleReqBatteryGeneralInfo(MessageProto& msg,
                                                            MessageBuffer& buffer)
{
    int ret = 0;

    ReqBatteryGeneralInfo bi = ReqBatteryGeneralInfo_init_zero;
    if (!msg.DecodeInnerMessage(ReqBatteryGeneralInfo_fields, &bi)) {
        return false;
    }

    BatteryGeneralData data;
    ret = battery_manager_.GetLastGeneralData(data);
    if (ret != 0) {
        logger_.wrn(
            "Failed to get last general battery sample. Responding with empty BatteryGeneralInfo.");
    }

    // Encode response
    ret = BatteryMessageEncoder::EncodeBatteryGeneralInfo(
        buffer, data.temp, data.volt, data.current, (int)data.remaining_capacity,
        (int)data.relative_charge_state, (int)data.cycle_count);
    if (!ret) {
        logger_.wrn("Failed to encode BatteryGeneralInfo.");
        return false;
    }
    buffer.msg_type = OUTGOING;

    return true;
}

/// @brief Add CPU as receiver of periodic battery information and encode a response.
bool BatteryMessageHandlerImpl::HandleReqBatteryNotifications(MessageProto& msg,
                                                              MessageBuffer& buffer)
{
    ReqBatteryNotifications bn = ReqBatteryNotifications_init_zero;
    if (!msg.DecodeInnerMessage(ReqBatteryNotifications_fields, &bn)) {
        return false;
    }

    if (bn.enable) {
        if (!battery_manager_.CpuIsSubscribed()) {
            battery_manager_.AddSubscriberGeneral([&](BatteryGeneralData sample_data) {
                msg_manager_.on_battery_gen_data_cb(sample_data);
            });
            battery_manager_.SetCpuSubscribed(true);
        }

        // Encode response
        if (!BatteryMessageEncoder::EncodeRespBatteryNotifications(buffer)) {
            logger_.wrn("Failed to Encode RespBatteryNotifications");
            return false;
        }
        buffer.msg_type = OUTGOING;
    }

    return true;
}

// Read status field from ResponseBatteryNotifications message
bool BatteryMessageHandlerImpl::HandleRespBatteryNotifications(MessageProto& msg,
                                                               MessageBuffer& buffer)
{
    RespBatteryNotifications bn = RespBatteryNotifications_init_zero;
    if (!msg.DecodeInnerMessage(RespBatteryNotifications_fields, &bn)) {
        return false;
    }

    return true;
}
