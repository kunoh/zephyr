#include "message_encoder.h"

namespace MessageEncoder {

bool EncodeOuterMessage(MessageBuffer &outer_buffer, MessageBuffer &inner_buffer, const char *type)
{
    pb_ostream_t stream;
    MessageOuter message = MessageOuter_init_zero;

    message.has_inner = true;
    strncpy(message.inner.type_url, type, sizeof(message.inner.type_url));
    if (inner_buffer.length > sizeof(message.inner.value.bytes)) {
        return false;
    }
    memcpy(message.inner.value.bytes, inner_buffer.data, inner_buffer.length);
    message.inner.value.size = inner_buffer.length;

    stream = pb_ostream_from_buffer(outer_buffer.data, sizeof(outer_buffer.data));

    if (!pb_encode(&stream, MessageOuter_fields, &message)) {
        return false;
    }
    outer_buffer.length = stream.bytes_written;

    return true;
}

bool EncodeInnerMessage(MessageBuffer &inner_buffer, const pb_msgdesc_t *fields,
                        const void *src_struct)
{
    pb_ostream_t stream;
    stream = pb_ostream_from_buffer(inner_buffer.data, sizeof(inner_buffer.data));
    if (!pb_encode(&stream, fields, src_struct)) {
        return false;
    }
    inner_buffer.length = stream.bytes_written;
    return true;
}

bool EncodeMessage(MessageBuffer &outer_buffer, const pb_msgdesc_t *fields, const void *src_struct,
                   const char *type)
{
    bool status;
    MessageBuffer inner_buffer;

    status = EncodeInnerMessage(inner_buffer, fields, src_struct);
    status |= EncodeOuterMessage(outer_buffer, inner_buffer, type);
    return status;
}

}  // namespace MessageEncoder