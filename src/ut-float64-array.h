#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_float64_array_new();

UtObject *ut_float64_array_new_with_elements(size_t length, ...);

UtObject *ut_float64_array_new_with_va_elements(size_t length, va_list ap);

double *ut_float64_array_get_data(UtObject *object);

bool ut_object_is_float64_array(UtObject *object);
