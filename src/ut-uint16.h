#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint16_new(uint16_t value);

uint16_t ut_uint16_get_value(UtObject *object);

bool ut_object_is_uint16(UtObject *object);