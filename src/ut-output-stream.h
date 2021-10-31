#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef void (*UtOutputStreamCallback)(void *user_data, UtObject *error);

typedef struct {
  void (*write)(UtObject *object, UtObject *data,
                UtOutputStreamCallback callback, void *user_data,
                UtObject *cancel);
} UtOutputStreamInterface;

extern int ut_output_stream_id;

void ut_output_stream_write(UtObject *object, UtObject *data);

void ut_output_stream_write_full(UtObject *object, UtObject *data,
                                 UtOutputStreamCallback callback,
                                 void *user_data, UtObject *cancel);

bool ut_object_implements_output_stream(UtObject *object);
