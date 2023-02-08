#include "display_message_handler.h"
#include "display.h"

class DisplayMessageHandlerImpl : public DisplayMessageHandler {
public:
    DisplayMessageHandlerImpl(Display& disp);

private:
    bool HandleRequestStopSpinner(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleResponseStopSpinner(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleRequestNewFrame(MessageProto& msg, MessageBuffer &buffer) override;
    bool HandleResponseNewFrame(MessageProto& msg, MessageBuffer &buffer) override;

private:
    Display& disp_;
};