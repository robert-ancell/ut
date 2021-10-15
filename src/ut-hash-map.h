#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_hash_map_new();

bool ut_object_is_hash_map(UtObject *object);
