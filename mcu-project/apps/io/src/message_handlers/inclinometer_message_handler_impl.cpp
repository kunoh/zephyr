#include "inclinometer_message_handler_impl.h"

LOG_MODULE_REGISTER(inclinometer_message_handler_impl, LOG_LEVEL_INF);

InclinometerMessageHandlerImpl::InclinometerMessageHandlerImpl(Inclinometer& incl)
    : incl_{incl}
{
}

bool InclinometerMessageHandlerImpl::HandleSetSampleRate(MessageProto& msg, MessageBuffer& buffer) {
    SetSampleRate sr = SetSampleRate_init_zero;
    if (!msg.DecodeInnerMessage(SetSampleRate_fields, &sr)){
        return false;
    }
    LOG_INF("Inc status: %d", incl_.status);
    incl_.write();
    return true;
}