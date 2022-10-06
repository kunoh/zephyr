#include "message_handler.h"

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