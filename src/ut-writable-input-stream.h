#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef void (*UtWritableInputStreamReadingCallback)(void *user_data,
                                                     UtObject *stream);

UtObject *ut_writable_input_stream_new();

void ut_writable_input_stream_set_reading_callback(
    UtObject *object, UtWritableInputStreamReadingCallback reading_callback,
    void *user_data, UtObject *cancel);

bool ut_writable_input_stream_get_reading(UtObject *object);

size_t ut_writable_input_stream_write(UtObject *object, UtObject *data,
                                      bool complete);

bool ut_object_is_writable_input_stream(UtObject *object);
