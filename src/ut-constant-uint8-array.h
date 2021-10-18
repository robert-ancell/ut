#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_constant_uint8_array_new(const uint8_t *data, size_t data_length);

bool ut_object_is_constant_uint8_array(UtObject *object);
