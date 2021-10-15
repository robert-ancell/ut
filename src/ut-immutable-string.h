#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_immutable_string_new(const char *text);

bool ut_object_is_immutable_string(UtObject *object);
