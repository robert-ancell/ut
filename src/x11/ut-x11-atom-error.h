#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_atom_error_new(uint32_t atom);

uint32_t ut_x11_atom_error_get_atom(UtObject *object);

bool ut_object_is_x11_atom_error(UtObject *object);
