#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef void (*UtFdStreamReadCallback)(void *user_data, UtObject *data);

typedef void (*UtFdStreamWriteCallback)(void *user_data, size_t n_written);

UtObject *ut_fd_stream_new(int fd);

void ut_fd_stream_read(UtObject *object, size_t count,
                       UtFdStreamReadCallback callback, void *user_data,
                       UtObject *cancel);

void ut_fd_stream_read_stream(UtObject *object, size_t block_size,
                              UtFdStreamReadCallback callback, void *user_data,
                              UtObject *cancel);

void ut_fd_stream_read_all(UtObject *object, size_t block_size,
                           UtFdStreamReadCallback callback, void *user_data,
                           UtObject *cancel);

void ut_fd_stream_write(UtObject *object, UtObject *data,
                        UtFdStreamWriteCallback callback, void *user_data,
                        UtObject *cancel);

void ut_fd_stream_write_all(UtObject *object, UtObject *data,
                            UtFdStreamWriteCallback callback, void *user_data,
                            UtObject *cancel);

bool ut_object_is_fd_stream(UtObject *object);
