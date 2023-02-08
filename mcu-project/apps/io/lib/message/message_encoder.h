#pragma once
#include <zephyr.h>
#include <logging/log.h>
#include <pb_encode.h>
#include "message_handler.h"

namespace MessageEncoder {

bool EncodeOuterMessage(MessageBuffer &outer_buffer, 
                    MessageBuffer &inner_buffer, 
                    const char* type);

bool EncodeInnerMessage(MessageBuffer &inner_buffer, 
                        const pb_msgdesc_t *fields, const void *src_struct);

bool EncodeMessage(MessageBuffer &outer_buffer, 
                   const pb_msgdesc_t *fields, const void *src_struct, const char* type);
}

