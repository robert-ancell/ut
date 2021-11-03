#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_unknown_error_new(uint8_t code, uint8_t major_opcode,
                                   uint16_t minor_opcode);

uint32_t ut_x11_unknown_error_get_code(UtObject *object);

uint8_t ut_x11_unknown_error_get_major_opcode(UtObject *object);

uint16_t ut_x11_unknown_error_get_minor_opcode(UtObject *object);

bool ut_object_is_x11_unknown_error(UtObject *object);
