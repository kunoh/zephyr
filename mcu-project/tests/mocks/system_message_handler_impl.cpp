#include "system_message_handler_impl.h"

LOG_MODULE_REGISTER(system_message_handler_impl, LOG_LEVEL_INF);

SystemMessageHandlerImpl::SystemMessageHandlerImpl()
{
}

bool SystemMessageHandlerImpl::HandleRequestVersion(MessageProto& msg, MessageBuffer& buffer) {
    RequestVersion rv = RequestVersion_init_zero;
    if (!msg.DecodeInnerMessage(RequestVersion_fields, &rv)){
        return false;
    }
    // TODO: Get Version
    int version = 2;
    if (!SystemMessageEncoder::EncodeResponseVersion(buffer, version)){
        LOG_WRN("Failed to Encode ResponseVersion\n");
    }
    return true;
}

bool SystemMessageHandlerImpl::HandleResponseVersion(MessageProto& msg, MessageBuffer& buffer) {
    ResponseVersion rv = ResponseVersion_init_zero;
    if (!msg.DecodeInnerMessage(ResponseVersion_fields, &rv)){
        return false;
    }
    return true;
}
