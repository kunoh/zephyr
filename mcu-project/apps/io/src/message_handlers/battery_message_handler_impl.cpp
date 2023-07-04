#include "battery_message_handler_impl.h"

#include "util.h"

LOG_MODULE_REGISTER(bat_msg_hdlr, CONFIG_BATTERY_MSG_HDLR_LOG_LEVEL);

BatteryMessageHandlerImpl::BatteryMessageHandlerImpl(BatteryManager& battery_manager,
                                                     MessageManager& msg_manager)
    : battery_manager_{battery_manager}, msg_manager_{msg_manager}
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

    ReqBatteryGeneralInfo bgi = ReqBatteryGeneralInfo_init_zero;
    if (!msg.DecodeInnerMessage(ReqBatteryGeneralInfo_fields, &bgi)) {
        return false;
    }

    BatteryGeneralData data;
    ret = battery_manager_.GetLastData(data);
    if (ret != 0) {
        LOG_WRN(
            "Failed to get last general battery sample. Responding with empty BatteryGeneralInfo.");
    }

    // Encode response
    if (!BatteryMessageEncoder::EncodeBatteryGeneralInfo(buffer, data.temp, data.volt, data.current,
                                                         (int)data.remaining_capacity,
                                                         (int)data.cycle_count)) {
        LOG_WRN("Failed to encode BatteryGeneralInfo.");
        return false;
    }

    buffer.msg_type = OUTGOING;
    return true;
}

/// @brief Have the battery manager get the last charging data battery sample
///        and encode it as a BatteryChargingInfo response to the CPU.
bool BatteryMessageHandlerImpl::HandleReqBatteryChargingInfo(MessageProto& msg,
                                                             MessageBuffer& buffer)
{
    int ret = 0;

    ReqBatteryChargingInfo bci = ReqBatteryChargingInfo_init_zero;
    if (!msg.DecodeInnerMessage(ReqBatteryChargingInfo_fields, &bci)) {
        return false;
    }

    BatteryChargingData data;
    ret = battery_manager_.GetLastData(data);
    if (ret != 0) {
        LOG_WRN(
            "Failed to get last battery charging data sample. Responding with empty "
            "BatteryChargingInfo.");
    }

    // Encode response
    if (!BatteryMessageEncoder::EncodeBatteryChargingInfo(
            buffer, data.status, data.des_chg_current, data.des_chg_volt,
            data.relative_charge_state, data.charging)) {
        LOG_WRN("Failed to encode BatteryChargingInfo.");
        return false;
    }

    buffer.msg_type = OUTGOING;
    return true;
}

/// @brief Add CPU as receiver of periodic battery information and encode a response.
bool BatteryMessageHandlerImpl::HandleReqBatteryNotifications(MessageProto& msg,
                                                              MessageBuffer& buffer)
{
    int ret = 0;
    ReqBatteryNotifications bn = ReqBatteryNotifications_init_zero;
    if (!msg.DecodeInnerMessage(ReqBatteryNotifications_fields, &bn)) {
        return false;
    }

    if (!strcmp(bn.data_type, "GENERAL")) {
        std::function<int(BatteryGeneralData)> func = [&](BatteryGeneralData data) {
            return msg_manager_.on_battery_gen_data_cb(data);
        };
        ret = battery_manager_.AddSubscriber(bn.subscription_type, bn.data_type, func);

    } else if (!strcmp(bn.data_type, "CHARGING")) {
        std::function<int(BatteryChargingData)> func = [&](BatteryChargingData data) {
            return msg_manager_.on_battery_chg_data_cb(data);
        };
        ret = battery_manager_.AddSubscriber(bn.subscription_type, bn.data_type, func);

    } else {
        LOG_WRN("Subscriber (%s) tried to sub to unknown data type (%s)", bn.subscription_type,
                bn.data_type);
        return false;
    }

    if (ret != 0 && ret != EEXIST) {
        LOG_WRN("Failed to subscribe (%s) to sub (%s) battery data", bn.subscription_type,
                bn.data_type);
        return false;
    }

    // Encode response
    if (!BatteryMessageEncoder::EncodeRespBatteryNotifications(buffer)) {
        LOG_WRN("Failed to Encode RespBatteryNotifications");
        return false;
    }
    buffer.msg_type = OUTGOING;

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

bool BatteryMessageHandlerImpl::HandleSetBatGenNotiThresh(MessageProto& msg, MessageBuffer& buffer)
{
    SetBatGenNotiThresh bgnt = SetBatGenNotiThresh_init_zero;
    if (!msg.DecodeInnerMessage(SetBatGenNotiThresh_fields, &bgnt)) {
        return false;
    }

    return true;
}

bool BatteryMessageHandlerImpl::HandleRespBatGenNotiThresh(MessageProto& msg, MessageBuffer& buffer)
{
    RespBatGenNotiThresh bgnt = RespBatGenNotiThresh_init_zero;
    if (!msg.DecodeInnerMessage(RespBatGenNotiThresh_fields, &bgnt)) {
        return false;
    }

    return true;
}

bool BatteryMessageHandlerImpl::HandleSetBatChgNotiThresh(MessageProto& msg, MessageBuffer& buffer)
{
    SetBatChgNotiThresh bcnt = SetBatChgNotiThresh_init_zero;
    if (!msg.DecodeInnerMessage(SetBatChgNotiThresh_fields, &bcnt)) {
        return false;
    }

    return true;
}

bool BatteryMessageHandlerImpl::HandleRespBatChgNotiThresh(MessageProto& msg, MessageBuffer& buffer)
{
    RespBatChgNotiThresh bcnt = RespBatChgNotiThresh_init_zero;
    if (!msg.DecodeInnerMessage(RespBatChgNotiThresh_fields, &bcnt)) {
        return false;
    }

    return true;
}

bool BatteryMessageHandlerImpl::HandleSetInstallationMode(MessageProto& msg, MessageBuffer& buffer)
{
    SetInstallationMode sim = SetInstallationMode_init_zero;
    if (!msg.DecodeInnerMessage(SetInstallationMode_fields, &sim)) {
        return false;
    }

    if (battery_manager_.SetInstallationMode(sim.mode) != 0) {
        LOG_WRN("Tried to set unknown installation mode.");
        return false;
    }
    // Encode response
    if (!BatteryMessageEncoder::EncodeRespInstallationMode(buffer)) {
        LOG_WRN("Failed to Encode RespInstallationMode");
        return false;
    }

    buffer.msg_type = OUTGOING;
    return true;
}

bool BatteryMessageHandlerImpl::HandleRespInstallationMode(MessageProto& msg, MessageBuffer& buffer)
{
    RespInstallationMode rim = RespInstallationMode_init_zero;
    if (!msg.DecodeInnerMessage(RespInstallationMode_fields, &rim)) {
        return false;
    }

    return true;
}

bool BatteryMessageHandlerImpl::HandleSetModeChargingLimit(MessageProto& msg, MessageBuffer& buffer)
{
    SetModeChargingLimit smcl = SetModeChargingLimit_init_zero;
    if (!msg.DecodeInnerMessage(SetModeChargingLimit_fields, &smcl)) {
        return false;
    }

    int ret = battery_manager_.SetModeChargingLimit(smcl.mode, smcl.chg_limit);
    if (ret != 0) {
        LOG_WRN("Failed to set charging limit. Reason: %d", ret);
        return false;
    }

    if (!BatteryMessageEncoder::EncodeRespModeChargingLimit(buffer))  // Encode response
    {
        LOG_WRN("Failed to Encode RespModeChargingLimit");
        return false;
    }

    buffer.msg_type = OUTGOING;
    return true;
}

bool BatteryMessageHandlerImpl::HandleRespModeChargingLimit(MessageProto& msg,
                                                            MessageBuffer& buffer)
{
    RespModeChargingLimit rmcl = RespModeChargingLimit_init_zero;
    if (!msg.DecodeInnerMessage(RespModeChargingLimit_fields, &rmcl)) {
        return false;
    }

    return true;
}