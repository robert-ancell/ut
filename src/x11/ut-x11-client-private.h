#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtX11ClientDecodeReplyFunction)(UtObject *object, uint8_t data0,
                                               UtObject *data);
typedef void (*UtX11ClientHandleErrorFunction)(UtObject *object,
                                               UtObject *error);

uint32_t ut_x11_client_create_resource_id(UtObject *object);

void ut_x11_client_send_request(UtObject *object, uint8_t opcode, uint8_t data0,
                                UtObject *data);

// Takes reference to callback_object
void ut_x11_client_send_request_with_reply(
    UtObject *object, uint8_t opcode, uint8_t data0, UtObject *data,
    UtX11ClientDecodeReplyFunction decode_reply_function,
    UtX11ClientHandleErrorFunction handle_error_function,
    UtObject *callback_object, UtObject *cancel);
