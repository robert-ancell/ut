#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint64_subarray_new(UtObject *parent, size_t start, size_t length);

bool ut_object_is_uint64_subarray(UtObject *object);
