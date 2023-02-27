#ifndef _EVENT_H
#define _EVENT_H

#define EVENT_QUEUE_SIZE 24

enum event_type { EVENT_BATTERY_UPDATE, EVENT_ERROR, EVENT_STATE_CHANGE };

struct event {
    enum event_type;
    union {
        int value
    };
};
int event_push(struct event *event);
int event_get(struct event *event);
#endif
