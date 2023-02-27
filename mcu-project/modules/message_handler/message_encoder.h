#pragma once
#include "message_handler.h"
#include "pb_encode.h"

namespace MessageEncoder {

bool EncodeOuterMessage(MessageBuffer &outer_buffer, MessageBuffer &inner_buffer, const char *type);

bool EncodeInnerMessage(MessageBuffer &inner_buffer, const pb_msgdesc_t *fields,
                        const void *src_struct);

bool EncodeMessage(MessageBuffer &outer_buffer, const pb_msgdesc_t *fields, const void *src_struct,
                   const char *type);
}  // namespace MessageEncoder
