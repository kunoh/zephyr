#include "display_message_handler_impl.h"

DisplayMessageHandlerImpl::DisplayMessageHandlerImpl(Logger& logger, DisplayManager& disp_mgr)
    : logger_{logger}, disp_mgr_{disp_mgr}
{}

bool DisplayMessageHandlerImpl::HandleRequestStopSpinner(MessageProto& msg, MessageBuffer& buffer)
{
    bool status = true;
    RequestStopSpinner ss = RequestStopSpinner_init_zero;
    if (!msg.DecodeInnerMessage(RequestStopSpinner_fields, &ss)) {
        return false;
    }
    disp_mgr_.StopSpinner();
    if (!DisplayMessageEncoder::EncodeResponseStopSpinner(buffer, status)) {
        logger_.wrn("Failed to Encode ResponseStopSpinner");
        return false;
    }
    return true;
}

bool DisplayMessageHandlerImpl::HandleResponseStopSpinner(MessageProto& msg, MessageBuffer& buffer)
{
    ResponseStopSpinner ss = ResponseStopSpinner_init_zero;
    if (!msg.DecodeInnerMessage(ResponseStopSpinner_fields, &ss)) {
        return false;
    }
    return true;
}

bool DisplayMessageHandlerImpl::HandleRequestNewFrame(MessageProto& msg, MessageBuffer& buffer)
{
    bool status = true;
    RequestNewFrame nf = RequestNewFrame_init_zero;
    if (!msg.DecodeInnerMessage(RequestNewFrame_fields, &nf)) {
        return false;
    }
    disp_mgr_.NextFrame();
    if (!DisplayMessageEncoder::EncodeResponseNewFrame(buffer, status)) {
        logger_.wrn("Failed to Encode ResponseNewFrame");
        return false;
    }
    return true;
}

bool DisplayMessageHandlerImpl::HandleResponseNewFrame(MessageProto& msg, MessageBuffer& buffer)
{
    ResponseNewFrame nf = ResponseNewFrame_init_zero;
    if (!msg.DecodeInnerMessage(ResponseNewFrame_fields, &nf)) {
        return false;
    }
    return true;
}