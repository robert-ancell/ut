#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_big_requests_extension_new(UtObject *client,
                                            uint8_t major_opcode);

bool ut_object_is_x11_big_requests_extension(UtObject *object);
