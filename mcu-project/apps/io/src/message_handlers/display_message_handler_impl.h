#pragma once
#include <display.h>
#include <logger.h>

#include "display_message_handler.h"

class DisplayMessageHandlerImpl : public DisplayMessageHandler {
public:
    DisplayMessageHandlerImpl(Logger& logger, Display& disp);

private:
    bool HandleRequestStopSpinner(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseStopSpinner(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRequestNewFrame(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseNewFrame(MessageProto& msg, MessageBuffer& buffer) override;

private:
    Logger& logger_;
    Display& disp_;
};