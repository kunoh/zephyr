#include "inclinometer_message_handler_impl.h"

LOG_MODULE_REGISTER(incl_msg_hdlr, CONFIG_INCLINOMETER_MSG_HDLR_LOG_LEVEL);

InclinometerMessageHandlerImpl::InclinometerMessageHandlerImpl(Inclinometer& incl) : incl_{incl}
{}

bool InclinometerMessageHandlerImpl::HandleSetSampleRate(MessageProto& msg, MessageBuffer& buffer)
{
    SetSampleRate sr = SetSampleRate_init_zero;
    if (!msg.DecodeInnerMessage(SetSampleRate_fields, &sr)) {
        return false;
    }
    incl_.Write();
    return true;
}