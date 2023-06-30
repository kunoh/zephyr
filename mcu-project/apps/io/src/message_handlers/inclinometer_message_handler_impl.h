#pragma once

#include <zephyr/logging/log.h>

#include "inclinometer_manager.h"
#include "inclinometer_message_handler.h"
#include "message_manager.h"

class InclinometerMessageHandlerImpl : public InclinometerMessageHandler {
public:
    InclinometerMessageHandlerImpl(InclinometerManager& incl);

private:
    bool HandleSetSamplePeriod(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRespSamplePeriod(MessageProto& msg, MessageBuffer& buffer) override;

    bool HandleGetSamplePeriod(MessageProto& msg, MessageBuffer& buffer) override;
    bool HandleRespGetSamplePeriod(MessageProto& msg, MessageBuffer& buffer) override;

    bool HandleGetAngle(MessageProto& msg, MessageBuffer& buffer) override;

    bool HandleXYZAngle(MessageProto& msg, MessageBuffer& buffer) override;

private:
    InclinometerManager& incl_;
};