#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_string_array_new();

UtObject *ut_string_array_new_with_elements(const char *value, ...);

UtObject *ut_string_array_new_with_va_elements(const char *value, va_list ap);

void ut_string_array_prepend(UtObject *object, const char *value);

void ut_string_array_append(UtObject *object, const char *value);

void ut_string_array_insert(UtObject *object, size_t index, const char *value);

bool ut_object_is_string_array(UtObject *object);
