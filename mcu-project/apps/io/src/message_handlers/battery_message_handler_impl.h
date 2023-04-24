#pragma once
#include <logger.h>

#include "battery_manager.h"
#include "battery_message_handler.h"
#include "message_manager.h"

class BatteryMessageHandlerImpl : public BatteryMessageHandler {
public:
    BatteryMessageHandlerImpl(Logger& logger, BatteryManager& battery_manager,
                              MessageManager& msg_manager);

private:
    bool HandleBatteryInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRequestBatteryInfo(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRequestBatteryNotifications(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseBatteryNotifications(MessageProto& msg, MessageBuffer& buffer) override;

    Logger& logger_;
    BatteryManager& battery_manager_;
    MessageManager& msg_manager_;
};
