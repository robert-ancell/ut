#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_zlib_error_new();

bool ut_object_is_zlib_error(UtObject *object);
