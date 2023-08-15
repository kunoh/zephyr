#pragma once

#include "commands.h"
#include "mgmt_handler.h"

namespace trackman {

class SystemMgmt : public MgmtHandler {
public:
    SystemMgmt();
    virtual ~SystemMgmt() = default;
    int Init();
    int HelloWorldPb(smp_streamer *ctxt);

private:
    mgmt_group mgmt_group_;
    mgmt_handler mgmt_group_handlers_[SYSTEM_NUMBER_OF_COMMANDS];
};

}  // namespace trackman