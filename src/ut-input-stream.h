#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef size_t (*UtInputStreamCallback)(void *user_data, UtObject *data,
                                        bool complete);

typedef struct {
  void (*read)(UtObject *object, UtInputStreamCallback callback,
               void *user_data, UtObject *cancel);
} UtInputStreamInterface;

extern int ut_input_stream_id;

void ut_input_stream_read(UtObject *object, UtInputStreamCallback callback,
                          void *user_data, UtObject *cancel);

void ut_input_stream_read_all(UtObject *object, UtInputStreamCallback callback,
                              void *user_data, UtObject *cancel);

UtObject *ut_input_stream_read_sync(UtObject *object);

bool ut_object_implements_input_stream(UtObject *object);
