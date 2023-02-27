#pragma once
#include <vector>

#include "message_handler.h"

class MessageDispatcher {
public:
    virtual void AddHandler(MessageHandler& h);
    virtual bool Handle(MessageProto& msg, MessageBuffer& buffer);

private:
    std::vector<MessageHandler*> handlers_;
};