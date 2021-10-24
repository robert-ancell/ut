#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_constant_utf8_string_new(const char *text);

bool ut_object_is_constant_utf8_string(UtObject *object);
