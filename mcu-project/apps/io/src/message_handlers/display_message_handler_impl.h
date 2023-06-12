#pragma once

#include <zephyr/logging/log.h>

#include "display_manager.h"
#include "display_message_handler.h"

class DisplayMessageHandlerImpl : public DisplayMessageHandler {
public:
    DisplayMessageHandlerImpl(DisplayManager& disp_mgr);

private:
    bool HandleRequestStopSpinner(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseStopSpinner(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRequestNewFrame(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseNewFrame(MessageProto& msg, MessageBuffer& buffer) override;

private:
    DisplayManager& disp_mgr_;
};