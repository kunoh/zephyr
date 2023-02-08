#pragma once
#include <zephyr.h>
#include <logging/log.h>
#include <sys/printk.h>
#include <cstring>
#include <vector>
#include <pb_decode.h>
#include "proto/outer.pb.h"

struct MessageBuffer {
    uint8_t data[MessageOuter_size];  
    size_t length = 0;
    bool is_received = false;
};

class MessageProto{
public:
    MessageOuter msg_outer = MessageOuter_init_zero;

public:
    bool DecodeOuterMessage(MessageBuffer &buffer);
    bool DecodeInnerMessage(const pb_msgdesc_t *fields, void *dest_struct);
};

class MessageHandler {
public:
    virtual bool Handle(MessageProto&, MessageBuffer &buffer) = 0;
};
