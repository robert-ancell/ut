#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_object_array_new();

UtObject *ut_object_array_new_with_data(size_t length, ...);

UtObject *ut_object_array_new_with_data_take(size_t length, ...);

bool ut_object_is_object_array(UtObject *object);
