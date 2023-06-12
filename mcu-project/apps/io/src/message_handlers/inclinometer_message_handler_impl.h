#pragma once

#include <zephyr/logging/log.h>

#include "inclinometer.h"
#include "inclinometer_message_handler.h"

class InclinometerMessageHandlerImpl : public InclinometerMessageHandler {
public:
    InclinometerMessageHandlerImpl(Inclinometer& incl);

private:
    bool HandleSetSampleRate(MessageProto& msg, MessageBuffer& buffer) override;

private:
    Inclinometer& incl_;
};