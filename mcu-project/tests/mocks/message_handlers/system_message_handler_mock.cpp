#include "system_message_handler_mock.h"

SystemMessageHandlerMock::SystemMessageHandlerMock(Logger& logger)
    : logger_{logger}
{
}

bool SystemMessageHandlerMock::HandleRequestVersion(MessageProto& msg, MessageBuffer& buffer) {
    RequestVersion rv = RequestVersion_init_zero;
    if (!msg.DecodeInnerMessage(RequestVersion_fields, &rv)){
        return false;
    }
    // TODO: Get Version
    int version = 2;
    if (!SystemMessageEncoder::EncodeResponseVersion(buffer, version)){
        logger_.wrn("Failed to Encode ResponseVersion\n");
    }
    buffer.msg_type = OUTGOING;
    
    return true;
}

bool SystemMessageHandlerMock::HandleResponseVersion(MessageProto& msg, MessageBuffer& buffer) {
    ResponseVersion rv = ResponseVersion_init_zero;
    if (!msg.DecodeInnerMessage(ResponseVersion_fields, &rv)){
        return false;
    }
    return true;
}
