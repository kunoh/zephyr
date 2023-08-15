#pragma once

#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/mgmt/mcumgr/smp/smp.h>

class Mgmt {
public:
    virtual ~Mgmt() = default;
    virtual int Init() = 0;
};