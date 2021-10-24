#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_constant_uint8_array_new(const uint8_t *data, size_t data_length);

const uint8_t *ut_constant_uint8_array_get_data(UtObject *object);

bool ut_object_is_constant_uint8_array(UtObject *object);
