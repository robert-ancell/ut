#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_input_stream_multiplexer_new(UtObject *input_stream);

UtObject *ut_input_stream_multiplexer_add(UtObject *object);

void ut_input_stream_multiplexer_set_active(UtObject *object, UtObject *stream);

bool ut_object_is_input_stream_multiplexer(UtObject *object);
