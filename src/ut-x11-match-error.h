#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_match_error_new();

bool ut_object_is_x11_match_error(UtObject *object);
