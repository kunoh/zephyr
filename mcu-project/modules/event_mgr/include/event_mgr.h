#ifndef __EVENT_MGR_H__
#define __EVENT_MGR_H__

#define EVENT_QUEUE_SIZE CONFIG_TM_EVENT_QUEUE_SIZE

enum event_type { EVENT_ERROR, EVENT_BATTERY_UPDATE, EVENT_SUSPEND, EVENT_IDLE };

struct event {
    enum event_type type;
    union {
        int value;
    };
};

int event_put(struct event *event_ptr);
int event_get(struct event *event_ptr);

#endif
