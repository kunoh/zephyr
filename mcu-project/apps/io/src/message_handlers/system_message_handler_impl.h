#pragma once

#include <zephyr/logging/log.h>

#include "system_message_handler.h"

class SystemMessageHandlerImpl : public SystemMessageHandler {
public:
    SystemMessageHandlerImpl();
    virtual ~SystemMessageHandlerImpl() = default;

private:
    bool HandleRequestVersion(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseVersion(MessageProto& msg, MessageBuffer& buffer) override;

    bool HandleRequestResetCause(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseResetCause(MessageProto& msg, MessageBuffer& buffer) override;

private:
};