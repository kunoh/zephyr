#pragma once

#include "display_manager.h"
#include "display_message_handler.h"
#include "logger.h"

class DisplayMessageHandlerImpl : public DisplayMessageHandler {
public:
    DisplayMessageHandlerImpl(Logger& logger, DisplayManager& disp_mgr);

private:
    bool HandleRequestStopSpinner(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseStopSpinner(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRequestNewFrame(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseNewFrame(MessageProto& msg, MessageBuffer& buffer) override;

private:
    Logger& logger_;
    DisplayManager& disp_mgr_;
};