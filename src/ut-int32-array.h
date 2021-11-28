#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_int32_array_new();

UtObject *ut_int32_array_new_with_data(size_t length, ...);

UtObject *ut_int32_array_new_with_va_data(size_t length, va_list ap);

int32_t *ut_int32_array_get_data(UtObject *object);

bool ut_object_is_int32_array(UtObject *object);
