#include "util.h"

#include <zephyr/drivers/hwinfo.h>

int GetResetInfo(uint32_t& supported_causes, uint32_t& reset_cause)
{
    int ret;
    ret = hwinfo_get_supported_reset_cause(&supported_causes);
    if (ret != 0) {
        return -ret;  // We want positive errno
    }

    ret = hwinfo_get_reset_cause(&reset_cause);
    return -ret;  // We want positive errno
}