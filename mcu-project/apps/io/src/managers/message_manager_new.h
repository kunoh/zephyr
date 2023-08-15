#pragma once

#include <vector>

#include "manager.h"
#include "mgmt_handler.h"
#include "util.h"

class MessageManagerNew : public Manager {
public:
    MessageManagerNew();
    virtual ~MessageManagerNew() = default;
    int Init() override;
    int Selftest() override;
    void AddErrorCb(void (*cb)(void*), void* user_data) override;
    void AddHandler(MgmtHandler& h);

private:
    std::vector<MgmtHandler*> mgmt_handlers_;
    CallbackWrapper on_error_;
};