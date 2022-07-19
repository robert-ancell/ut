#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_object_array_new();

UtObject *ut_object_array_new_with_elements(UtObject *item0, ...);

UtObject *ut_object_array_new_with_elements_take(UtObject *item0, ...);

bool ut_object_is_object_array(UtObject *object);
