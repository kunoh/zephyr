#include "system_message_handler_impl.h"

LOG_MODULE_REGISTER(sys_msg_hdlr, CONFIG_SYSTEM_MSG_HDLR_LOG_LEVEL);

SystemMessageHandlerImpl::SystemMessageHandlerImpl()
{}

bool SystemMessageHandlerImpl::HandleRequestVersion(MessageProto& msg, MessageBuffer& buffer)
{
    LOG_INF("ResponseVersion Received");

    RequestVersion rv = RequestVersion_init_zero;
    if (!msg.DecodeInnerMessage(RequestVersion_fields, &rv)) {
        return false;
    }
    // TODO: Get Version
    int version = 2;
    if (!SystemMessageEncoder::EncodeResponseVersion(buffer, version)) {
        LOG_WRN("Failed to Encode ResponseVersion");
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
