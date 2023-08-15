#include "system_mgmt.h"

#include <zephyr/logging/log.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/sys/__assert.h>

#include "message_encoder.h"
#include "system_message_encoder.h"

LOG_MODULE_REGISTER(system_mgmt, CONFIG_SYSTEM_MGMT_LOG_LEVEL);

namespace trackman {
namespace {
static SystemMgmt *g_sys_mgmt = nullptr;

static int GhelloWorldPb(smp_streamer *ctxt)
{
    if (!g_sys_mgmt) {
        LOG_ERR("SystemMgmt is not initialized yet");
        return -1;
    }

    return g_sys_mgmt->HelloWorldPb(ctxt);
}
}  // namespace

SystemMgmt::SystemMgmt()
{
    g_sys_mgmt = this;
}

int SystemMgmt::Init()
{
    mgmt_group_handlers_[SYSTEM_MGMT_ID_HELLO_PB] = {.mh_read = &GhelloWorldPb, .mh_write = NULL};

    mgmt_group_ = {
        .mg_handlers = mgmt_group_handlers_,
        .mg_handlers_count = SYSTEM_NUMBER_OF_COMMANDS,
        .mg_group_id = TM_MGMT_GROUP_ID_SYSTEM,
#if defined(CONFIG_MCUMGR_MGMT_CUSTOM_PAYLOAD)
        .custom_payload = true,
#endif
    };

    mgmt_register_group(&mgmt_group_);
    return 0;
}

int SystemMgmt::HelloWorldPb(smp_streamer *ctxt)
{
    zcbor_state_t *zse = ctxt->writer->zs;

    MessageBuffer inner_buffer;
    ResponseVersion message = ResponseVersion_init_zero;
    message.version = 5;
    MessageEncoder::EncodeInnerMessage(inner_buffer, ResponseVersion_fields, &message);

    memmove(zse->payload_mut, inner_buffer.data, inner_buffer.length);
    zse->payload += inner_buffer.length;

    return 0;
}
}  // namespace trackman