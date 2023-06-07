#pragma once

#include "logger.h"
#include "system_message_handler.h"

class SystemMessageHandlerImpl : public SystemMessageHandler {
public:
    SystemMessageHandlerImpl(Logger& logger);
    virtual ~SystemMessageHandlerImpl() = default;

private:
    bool HandleRequestVersion(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleResponseVersion(MessageProto& msg, MessageBuffer& buffer) override;

private:
    Logger& logger_;
};