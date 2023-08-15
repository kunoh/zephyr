#pragma once

#include "outer.pb.h"
#include "pb_decode.h"

typedef enum {
    INCOMING,
    OUTGOING,
} msg_type_t;

struct MessageBuffer {
    uint8_t data[MessageOuter_size];
    size_t length = 0;
    msg_type_t msg_type = INCOMING;
};

class MessageProto {
public:
    MessageOuter msg_outer = MessageOuter_init_zero;

public:
    bool DecodeOuterMessage(MessageBuffer &buffer);
    bool DecodeInnerMessage(const pb_msgdesc_t *fields, void *dest_struct);
};

class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    virtual bool Handle(MessageProto &, MessageBuffer &buffer) = 0;
};
