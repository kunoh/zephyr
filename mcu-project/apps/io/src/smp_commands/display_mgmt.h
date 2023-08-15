#pragma once

#include "commands.h"
#include "display_manager.h"
#include "mgmt.h"

namespace trackman {

class DisplayMgmt : public Mgmt {
public:
    DisplayMgmt(DisplayManager& disp_mgr);
    virtual ~DisplayMgmt() = default;
    int Init();
    int NextFrame(smp_streamer* ctxt);
    int StopSpinner(smp_streamer* ctxt);

private:
    DisplayManager& disp_mgr_;
    mgmt_group mgmt_group_;
    mgmt_handler mgmt_group_handlers_[DISPLAY_NUMBER_OF_COMMANDS];
};

}  // namespace trackman