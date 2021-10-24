#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_utf8_string_new(const char *text);

UtObject *ut_utf8_string_new_sized(const char *text, size_t length);

bool ut_object_is_utf8_string(UtObject *object);
