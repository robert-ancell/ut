#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_writable_input_stream_new();

size_t ut_writable_input_stream_write(UtObject *object, UtObject *data,
                                      bool complete);

bool ut_object_is_writable_input_stream(UtObject *object);
