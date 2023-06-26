#include <event_mgr.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(event_mgr);

// sizeof struct event should be a multiplication of a align boundary (4)
K_MSGQ_DEFINE(event_q, sizeof(struct event), EVENT_QUEUE_SIZE, 4);

int event_put(struct event *evt_ptr)
{
    return k_msgq_put(&event_q, evt_ptr, K_NO_WAIT);
}

int event_get(struct event *event_ptr)
{
    return k_msgq_get(&event_q, event_ptr, K_FOREVER);
}
