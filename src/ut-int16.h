#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_int16_new(int16_t value);

int16_t ut_int16_get_value(UtObject *object);

bool ut_object_is_int16(UtObject *object);