#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_object_array_new();

bool ut_object_is_object_array(UtObject *object);
