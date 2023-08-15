#include "message_manager_new.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(msg_mgr_new, CONFIG_MESSAGE_MANAGER_LOG_LEVEL);

MessageManagerNew::MessageManagerNew()
{}

int MessageManagerNew::Init()
{
    int res = 0;
    for (auto mh : mgmt_handlers_) {
        res = mh->Init();
        if (res != 0) {
            LOG_ERR("Message Manager Initialization Failed");
            return 1;
        }
    }
    return 0;
}

int MessageManagerNew::Selftest()
{
    LOG_INF("Message Manager Selftest OK");
    return 0;
}

void MessageManagerNew::AddErrorCb(void (*cb)(void*), void* user_data)
{
    on_error_.cb = cb;
    on_error_.user_data = user_data;
}

void MessageManagerNew::AddHandler(MgmtHandler& h)
{
    mgmt_handlers_.push_back(&h);
}