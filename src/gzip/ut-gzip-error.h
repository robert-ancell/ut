#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_gzip_error_new();

bool ut_object_is_gzip_error(UtObject *object);
