#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_immutable_uint8_list_new(const uint8_t *data, size_t data_length);

bool ut_object_is_immutable_uint8_list(UtObject *object);
