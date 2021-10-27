#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_float64_array_new();

UtObject *ut_float64_array_new_with_data(size_t length, ...);

UtObject *ut_float64_array_new_with_va_data(size_t length, va_list ap);

void ut_float64_array_append(UtObject *object, double data);

void ut_float64_array_append_block(UtObject *object, const double *data,
                                   size_t data_length);

void ut_float64_array_insert(UtObject *object, size_t index, double data);

void ut_float64_array_insert_block(UtObject *object, size_t index,
                                   const double *data, size_t data_length);

double *ut_float64_array_get_data(UtObject *object);

bool ut_object_is_float64_array(UtObject *object);
