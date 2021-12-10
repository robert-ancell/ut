#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef void (*UtX11ClientPresentEnableCallback)(void *user_data,
                                                 UtObject *error);

UtObject *ut_x11_present_extension_new(UtObject *client, uint8_t major_opcode);

void ut_x11_present_extension_enable(UtObject *object,
                                     UtX11ClientPresentEnableCallback callback,
                                     void *user_data, UtObject *cancel);

bool ut_object_is_x11_present_extension(UtObject *object);
