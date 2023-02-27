
#include "message_dispatcher.h"

void MessageDispatcher::AddHandler(MessageHandler& h)
{
    handlers_.push_back(&h);
}

bool MessageDispatcher::Handle(MessageProto& msg, MessageBuffer& buffer)
{
    for (auto h : handlers_) {
        if (h->Handle(msg, buffer)) {
            return true;
        }
    }
    return false;
}