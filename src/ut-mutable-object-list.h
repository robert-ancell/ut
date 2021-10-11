#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_mutable_object_list_new();

bool ut_object_is_mutable_object_list(UtObject *object);
