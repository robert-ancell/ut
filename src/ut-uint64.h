#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint64_new(uint64_t value);

uint64_t ut_uint64_get_value(UtObject *object);

bool ut_object_is_uint64(UtObject *object);
