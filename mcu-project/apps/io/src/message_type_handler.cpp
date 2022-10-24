#include "message_handler.h"

extern "C" void display_stop_spinner(void);
extern "C" void display_signal_new_frame(void);

LOG_MODULE_REGISTER(message_type_handler, LOG_LEVEL_INF);

void RequestVersionMessageHandler::HandleRequestVersion(const RequestVersion &rv, MessageBuffer &buffer) {
    // TODO: Get Version
    int version = 2;

    if (!MessageEncoder::EncodeResponseVersion(buffer, version)){
        LOG_WRN("Failed to Encode ResponseVersion\n");
    }
}

void ResponseVersionMessageHandler::HandleResponseVersion(const ResponseVersion &rv, MessageBuffer &buffer) {
    return;
}


void RequestStopSpinnerMessageHandler::HandleRequestStopSpinner(const RequestStopSpinner &rv, MessageBuffer &buffer) {
    bool status = true;
	display_stop_spinner();

    if (!MessageEncoder::EncodeResponseStopSpinner(buffer, status)){
        LOG_WRN("Failed to Encode ResponseStopSpinner\n");
    }
}

void ResponseStopSpinnerMessageHandler::HandleResponseStopSpinner(const ResponseStopSpinner &rv, MessageBuffer &buffer) {
    return;
}


void RequestNewFrameMessageHandler::HandleRequestNewFrame(const RequestNewFrame &rv, MessageBuffer &buffer) {
    bool status = true;
	display_signal_new_frame();

    if (!MessageEncoder::EncodeResponseNewFrame(buffer, status)){
        LOG_WRN("Failed to Encode ResponseNewFrame\n");
    }
}

void ResponseNewFrameMessageHandler::HandleResponseNewFrame(const ResponseNewFrame &rv, MessageBuffer &buffer) {
    return;
}