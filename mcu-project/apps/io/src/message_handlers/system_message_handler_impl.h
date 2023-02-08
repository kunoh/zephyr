#include "system_message_handler.h"
#include "proto/system.pb.h"

class SystemMessageHandlerImpl : public SystemMessageHandler {
public:
    SystemMessageHandlerImpl();

private:
    bool HandleRequestVersion(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleResponseVersion(MessageProto& msg, MessageBuffer &buffer) override;

};