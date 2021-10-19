#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_float64_new(double value);

double ut_float64_get_value(UtObject *object);

bool ut_object_is_float64(UtObject *object);
