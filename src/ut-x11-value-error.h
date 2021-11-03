#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_value_error_new(uint32_t value);

uint32_t ut_x11_value_error_get_value(UtObject *object);

bool ut_object_is_x11_value_error(UtObject *object);
