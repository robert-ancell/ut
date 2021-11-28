#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint8_array_with_fds_new(UtObject *data, UtObject *fds);

UtObject *ut_uint8_array_with_fds_get_data(UtObject *object);

UtObject *ut_uint8_array_with_fds_get_fds(UtObject *object);

bool ut_object_is_uint8_array_with_fds(UtObject *object);
