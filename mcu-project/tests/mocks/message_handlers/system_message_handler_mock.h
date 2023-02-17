#pragma once
#include <logger.h>
#include "system_message_handler.h"

class SystemMessageHandlerMock : public SystemMessageHandler {
public:
    SystemMessageHandlerMock(Logger& logger);
    virtual ~SystemMessageHandlerMock() = default;

private:
    bool HandleRequestVersion(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleResponseVersion(MessageProto& msg, MessageBuffer &buffer) override;

private:
    Logger& logger_;
};