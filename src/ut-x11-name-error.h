#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_name_error_new();

bool ut_object_is_x11_name_error(UtObject *object);
