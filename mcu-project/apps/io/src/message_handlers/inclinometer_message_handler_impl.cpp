#include "inclinometer_message_handler_impl.h"

InclinometerMessageHandlerImpl::InclinometerMessageHandlerImpl(Logger& logger, Inclinometer& incl)
    : logger_{logger}, incl_{incl}
{}

bool InclinometerMessageHandlerImpl::HandleSetSampleRate(MessageProto& msg, MessageBuffer& buffer)
{
    SetSampleRate sr = SetSampleRate_init_zero;
    if (!msg.DecodeInnerMessage(SetSampleRate_fields, &sr)) {
        return false;
    }
    incl_.write();
    return true;
}