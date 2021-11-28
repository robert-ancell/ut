#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_mit_shm_extension_new(UtObject *client, uint8_t major_opcode,
                                       uint8_t first_event,
                                       uint8_t first_error);

bool ut_object_is_x11_mit_shm_extension(UtObject *object);
