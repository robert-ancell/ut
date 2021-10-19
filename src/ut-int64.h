#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_int64_new(int64_t value);

int64_t ut_int64_get_value(UtObject *object);

bool ut_object_is_int64(UtObject *object);
