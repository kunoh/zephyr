#include "display_mgmt.h"

#include <zephyr/logging/log.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/sys/__assert.h>

#include "display_message_encoder.h"
#include "message_encoder.h"

LOG_MODULE_REGISTER(disp_mgmt, CONFIG_DISPLAY_MGMT_LOG_LEVEL);

namespace trackman {
namespace {
static DisplayMgmt *g_disp_mgmt = nullptr;

static int GnextFrame(smp_streamer *ctxt)
{
    if (!g_disp_mgmt) {
        LOG_ERR("DisplayMgmt is not initialized yet");
        return -1;
    }

    return g_disp_mgmt->NextFrame(ctxt);
}

static int GstopSpinner(smp_streamer *ctxt)
{
    if (!g_disp_mgmt) {
        LOG_ERR("DisplayMgmt is not initialized yet");
        return -1;
    }

    return g_disp_mgmt->StopSpinner(ctxt);
}
}  // namespace

DisplayMgmt::DisplayMgmt(DisplayManager &disp_mgr) : disp_mgr_{disp_mgr}
{
    g_disp_mgmt = this;
}

int DisplayMgmt::Init()
{
    mgmt_group_handlers_[DISPLAY_MGMT_ID_NEXT_FRAME] = {.mh_read = &GnextFrame, .mh_write = NULL};

    mgmt_group_handlers_[DISPLAY_MGMT_ID_STOP_SPINNER] = {.mh_read = &GstopSpinner,
                                                          .mh_write = NULL};

    mgmt_group_ = {
        .mg_handlers = mgmt_group_handlers_,
        .mg_handlers_count = DISPLAY_NUMBER_OF_COMMANDS,
        .mg_group_id = TM_MGMT_GROUP_ID_DISPLAY,
#if defined(CONFIG_MCUMGR_MGMT_CUSTOM_PAYLOAD)
        .custom_payload = true,
#endif
    };

    mgmt_register_group(&mgmt_group_);
    return 0;
}

int DisplayMgmt::NextFrame(smp_streamer *ctxt)
{
    disp_mgr_.NextFrame();
    return 0;
}

int DisplayMgmt::StopSpinner(smp_streamer *ctxt)
{
    disp_mgr_.StopSpinner();
    return 0;
}
}  // namespace trackman