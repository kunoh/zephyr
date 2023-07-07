#include "system_message_handler_impl.h"

#include "util.h"

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

bool SystemMessageHandlerImpl::HandleRequestResetCause(MessageProto& msg, MessageBuffer& buffer)
{
    RequestResetCause rc = RequestResetCause_init_zero;
    if (!msg.DecodeInnerMessage(RequestResetCause_fields, &rc)) {
        return false;
    }

    int ret;
    uint32_t supported_causes = 0;
    uint32_t reset_cause = 0;
    ret = GetResetInfo(supported_causes, reset_cause);
    if (ret != 0) {
        LOG_WRN("Failed to get reset cause");
        return false;
    }

    if (!SystemMessageEncoder::EncodeResponseResetCause(buffer, (int)supported_causes,
                                                        reset_cause)) {
        LOG_WRN("Failed to Encode ResponseResetCause");
    }
    buffer.msg_type = OUTGOING;

    return true;
}

bool SystemMessageHandlerImpl::HandleResponseResetCause(MessageProto& msg, MessageBuffer& buffer)
{
    ResponseResetCause rc = ResponseResetCause_init_zero;
    if (!msg.DecodeInnerMessage(ResponseResetCause_fields, &rc)) {
        return false;
    }

    return true;
}