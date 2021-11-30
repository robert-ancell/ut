#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_open_type_file_new(UtObject *data);

void ut_open_type_file_open(UtObject *object);

bool ut_object_is_open_type_file(UtObject *object);
