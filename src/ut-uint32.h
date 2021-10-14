#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint32_new(uint32_t value);

uint32_t ut_uint32_get_value(UtObject *object);

bool ut_object_is_uint32(UtObject *object);