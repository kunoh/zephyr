#include "display_message_handler_impl.h"

DisplayMessageHandlerImpl::DisplayMessageHandlerImpl(Logger& logger, Display& disp)
    : logger_{logger}
    , disp_{disp}
{
}

bool DisplayMessageHandlerImpl::HandleRequestStopSpinner(MessageProto& msg, MessageBuffer& buffer) {
    bool status = true;
    RequestStopSpinner ss = RequestStopSpinner_init_zero;
        if (!msg.DecodeInnerMessage(RequestStopSpinner_fields, &ss)){
        return false;
    }
    //Saving this comment for later use
    //display_stop_spinner();
    if (!DisplayMessageEncoder::EncodeResponseStopSpinner(buffer, status)){
        logger_.wrn("Failed to Encode ResponseStopSpinner\n");
        return false;
    }
    return true;
}

bool DisplayMessageHandlerImpl::HandleResponseStopSpinner(MessageProto& msg, MessageBuffer& buffer) {
    ResponseStopSpinner ss = ResponseStopSpinner_init_zero;
    if (!msg.DecodeInnerMessage(ResponseStopSpinner_fields, &ss)){
        return false;
    }
    return true;
}

bool DisplayMessageHandlerImpl::HandleRequestNewFrame(MessageProto& msg, MessageBuffer& buffer) {
    bool status = true;
    RequestNewFrame nf = RequestNewFrame_init_zero;
    if (!msg.DecodeInnerMessage(RequestNewFrame_fields, &nf)){
        return false;
    }
    //Saving this comment for later use
    //display_signal_new_frame();
    if (!DisplayMessageEncoder::EncodeResponseNewFrame(buffer, status)){
        logger_.wrn("Failed to Encode ResponseNewFrame\n");
        return false;
    }
    return true;
}

bool DisplayMessageHandlerImpl::HandleResponseNewFrame(MessageProto& msg, MessageBuffer& buffer) {
    ResponseNewFrame nf = ResponseNewFrame_init_zero;
    if (!msg.DecodeInnerMessage(ResponseNewFrame_fields, &nf)){
        return false;
    }
    return true;
}