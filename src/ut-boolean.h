#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_boolean_new(bool value);

bool ut_boolean_get_value(UtObject *object);

bool ut_object_is_boolean(UtObject *object);
