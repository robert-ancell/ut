#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_local_file_new(const char *path);

UtObject *ut_local_file_get_fd(UtObject *object);

bool ut_object_is_local_file(UtObject *object);
