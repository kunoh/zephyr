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

    ReqBatteryGeneralInfo bi = ReqBatteryGeneralInfo_init_zero;
    if (!msg.DecodeInnerMessage(ReqBatteryGeneralInfo_fields, &bi)) {
        return false;
    }

    BatteryGeneralData data;
    ret = battery_manager_.GetLastGeneralData(data);
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
            LOG_WRN("Failed to Encode RespBatteryNotifications");
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

bool BatteryMessageHandlerImpl::HandleSetInstallationMode(MessageProto& msg, MessageBuffer& buffer)
{
    SetInstallationMode sim = SetInstallationMode_init_zero;
    if (!msg.DecodeInnerMessage(SetInstallationMode_fields, &sim)) {
        return false;
    } else if (!battery_manager_.ModeIsKnown(sim.mode)) {
        LOG_WRN("Tried to set non-existing installation mode.");
        return false;
    }

    battery_manager_.SetInstallationMode((installation_mode_t)sim.mode);
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
    } else if (!battery_manager_.ModeIsKnown(smcl.mode)) {
        LOG_WRN("Tried to set non-existing installation mode.");
        return false;
    }

    if (battery_manager_.SetModeChargingLimit((installation_mode_t)smcl.mode, smcl.chg_limit) !=
        0) {
        LOG_WRN("Failed to set charging limit. Limit outside permissible range.");
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