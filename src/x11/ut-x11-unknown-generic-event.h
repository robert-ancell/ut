#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_unknown_generic_event_new(uint8_t major_opcode, uint16_t code);

uint8_t ut_x11_unknown_generic_event_get_major_opcode(UtObject *object);

uint16_t ut_x11_unknown_generic_event_get_code(UtObject *object);

bool ut_object_is_x11_unknown_generic_event(UtObject *object);
