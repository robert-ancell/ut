#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_true_type_error_new();

bool ut_object_is_true_type_error(UtObject *object);
