#include "message_handler.h"

bool MessageProto::DecodeOuterMessage(MessageBuffer &buffer)
{
    bool status;

    pb_istream_t stream = pb_istream_from_buffer(buffer.data, buffer.length);

    status = pb_decode(&stream, MessageOuter_fields, &msg_outer);

    return status;
}

bool MessageProto::DecodeInnerMessage(const pb_msgdesc_t *fields, void *dest_struct)
{
    bool status;

    pb_istream_t stream =
        pb_istream_from_buffer(msg_outer.inner.value.bytes, msg_outer.inner.value.size);

    status = pb_decode(&stream, fields, dest_struct);

    return status;
}