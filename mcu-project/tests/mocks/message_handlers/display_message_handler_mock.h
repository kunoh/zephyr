#pragma once
#include <logger.h>
#include <display.h>
#include "display_message_handler.h"


class DisplayMessageHandlerMock : public DisplayMessageHandler {
public:
    DisplayMessageHandlerMock(Logger& logger, Display& disp);

private:
    bool HandleRequestStopSpinner(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleResponseStopSpinner(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleRequestNewFrame(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleResponseNewFrame(MessageProto& msg, MessageBuffer &buffer) override;

private:
    Logger& logger_;
    Display& disp_;
};