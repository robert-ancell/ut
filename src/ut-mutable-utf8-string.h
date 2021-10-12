#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_mutable_utf8_string_new(const char *text);

bool ut_object_is_mutable_uint8_list(UtObject *object);
