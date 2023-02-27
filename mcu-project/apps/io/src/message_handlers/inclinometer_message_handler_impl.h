#pragma once
#include <inclinometer.h>
#include <logger.h>

#include "inclinometer_message_handler.h"

class InclinometerMessageHandlerImpl : public InclinometerMessageHandler {
public:
    InclinometerMessageHandlerImpl(Logger& logger, Inclinometer& incl);

private:
    bool HandleSetSampleRate(MessageProto& msg, MessageBuffer& buffer) override;

private:
    Logger& logger_;
    Inclinometer& incl_;
};