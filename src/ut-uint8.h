#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint8_new(uint8_t value);

uint8_t ut_uint8_get_value(UtObject *object);

bool ut_object_is_uint8(UtObject *object);