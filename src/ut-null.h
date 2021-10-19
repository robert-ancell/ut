#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_null_new();

bool ut_object_is_null(UtObject *object);
