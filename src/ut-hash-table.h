#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_hash_table_new();

bool ut_object_is_hash_table(UtObject *object);
