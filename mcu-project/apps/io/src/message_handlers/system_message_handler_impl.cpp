#include "system_message_handler_impl.h"

SystemMessageHandlerImpl::SystemMessageHandlerImpl(Logger& logger) : logger_{logger}
{}

bool SystemMessageHandlerImpl::HandleRequestVersion(MessageProto& msg, MessageBuffer& buffer)
{
    logger_.inf("ResponseVersion Received");

    RequestVersion rv = RequestVersion_init_zero;
    if (!msg.DecodeInnerMessage(RequestVersion_fields, &rv)) {
        return false;
    }
    // TODO: Get Version
    int version = 2;
    if (!SystemMessageEncoder::EncodeResponseVersion(buffer, version)) {
        logger_.wrn("Failed to Encode ResponseVersion");
    }
    buffer.msg_type = OUTGOING;

    return true;
}

bool SystemMessageHandlerImpl::HandleResponseVersion(MessageProto& msg, MessageBuffer& buffer)
{
    ResponseVersion rv = ResponseVersion_init_zero;
    if (!msg.DecodeInnerMessage(ResponseVersion_fields, &rv)) {
        return false;
    }
    return true;
}
