#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_general_error_new(const char *description);

const char *ut_general_error_get_description(UtObject *object);

bool ut_object_is_general_error(UtObject *object);
