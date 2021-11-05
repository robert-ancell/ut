#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_int32_new(int32_t value);

int32_t ut_int32_get_value(UtObject *object);

bool ut_object_is_int32(UtObject *object);