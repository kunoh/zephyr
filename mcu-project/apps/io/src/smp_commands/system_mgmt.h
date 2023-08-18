#pragma once

#include "commands.h"
#include "mgmt.h"

namespace trackman {

class SystemMgmt : public Mgmt {
public:
    SystemMgmt();
    virtual ~SystemMgmt() = default;
    int Init();
    int HelloWorldPb(smp_streamer *ctxt);
    int ResetCausePb(smp_streamer *ctxt);

private:
    mgmt_group mgmt_group_;
    mgmt_handler mgmt_group_handlers_[SYSTEM_NUMBER_OF_COMMANDS];
};

}  // namespace trackman