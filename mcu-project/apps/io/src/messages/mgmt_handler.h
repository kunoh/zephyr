#pragma once

#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/mgmt/mcumgr/smp/smp.h>

class MgmtHandler {
public:
    virtual ~MgmtHandler() = default;
    virtual int Init() = 0;
};