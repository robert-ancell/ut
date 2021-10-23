#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_end_of_stream_new(UtObject *unused_data);

UtObject *ut_end_of_stream_get_unused_data(UtObject *object);

bool ut_object_is_end_of_stream(UtObject *object);
