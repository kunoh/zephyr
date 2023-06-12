#pragma once

#include <zephyr/logging/log.h>

#include "battery_manager.h"
#include "battery_message_handler.h"
#include "message_manager.h"

class BatteryMessageHandlerImpl : public BatteryMessageHandler {
public:
    BatteryMessageHandlerImpl(BatteryManager& battery_manager, MessageManager& msg_manager);

private:
    bool HandleBatteryGeneralInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleBatteryChargingInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleReqBatteryGeneralInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleReqBatteryNotifications(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRespBatteryNotifications(MessageProto& msg, MessageBuffer& buffer) override;

    bool HandleSetInstallationMode(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRespInstallationMode(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleSetModeChargingLimit(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRespModeChargingLimit(MessageProto& msg, MessageBuffer& buffer) override;

    BatteryManager& battery_manager_;
    MessageManager& msg_manager_;
};
