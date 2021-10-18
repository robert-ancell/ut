#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_constant_string_new(const char *text);

bool ut_object_is_constant_string(UtObject *object);
