#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_png_error_new();

bool ut_object_is_png_error(UtObject *object);
