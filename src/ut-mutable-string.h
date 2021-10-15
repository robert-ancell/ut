#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_mutable_string_new(const char *text);

void ut_mutable_string_clear(UtObject *object);

void ut_mutable_string_prepend(UtObject *object, const char *text);

void ut_mutable_string_append(UtObject *object, const char *text);

bool ut_object_is_mutable_string(UtObject *object);
