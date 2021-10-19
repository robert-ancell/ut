#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef size_t (*UtInputStreamCallback)(void *user_data, UtObject *data);

typedef struct {
  void (*read)(UtObject *object, size_t block_size,
               UtInputStreamCallback callback, void *user_data,
               UtObject *cancel);

  void (*read_all)(UtObject *object, size_t block_size,
                   UtInputStreamCallback callback, void *user_data,
                   UtObject *cancel);
} UtInputStreamFunctions;

extern int ut_input_stream_id;

void ut_input_stream_read(UtObject *object, size_t block_size,
                          UtInputStreamCallback callback, void *user_data,
                          UtObject *cancel);

void ut_input_stream_read_all(UtObject *object, size_t block_size,
                              UtInputStreamCallback callback, void *user_data,
                              UtObject *cancel);

bool ut_object_implements_input_stream(UtObject *object);
