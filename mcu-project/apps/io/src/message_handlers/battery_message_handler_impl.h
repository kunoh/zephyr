#pragma once

#include "battery_manager.h"
#include "battery_message_handler.h"
#include "logger.h"
#include "message_manager.h"

class BatteryMessageHandlerImpl : public BatteryMessageHandler {
public:
    BatteryMessageHandlerImpl(Logger& logger, BatteryManager& battery_manager,
                              MessageManager& msg_manager);

private:
    bool HandleBatteryGeneralInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleBatteryChargingInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleReqBatteryGeneralInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleReqBatteryNotifications(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRespBatteryNotifications(MessageProto& msg, MessageBuffer& buffer) override;

    Logger& logger_;
    BatteryManager& battery_manager_;
    MessageManager& msg_manager_;
};
