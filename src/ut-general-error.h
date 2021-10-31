#include <stdarg.h>
#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_general_error_new(const char *format, ...)
    __attribute((format(printf, 1, 2)));

UtObject *ut_general_error_new_valist(const char *format, va_list ap);

UtObject *ut_general_error_new_literal(const char *description);

const char *ut_general_error_get_description(UtObject *object);

bool ut_object_is_general_error(UtObject *object);
