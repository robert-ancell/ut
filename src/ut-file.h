#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef void (*UtFileReadCallback)(void *user_data, UtObject *data);

UtObject *ut_file_new(const char *path);

void ut_file_open_read(UtObject *object);

void ut_file_read(UtObject *object, size_t count, UtFileReadCallback callback,
                  void *user_data, UtObject *cancel);

void ut_file_close(UtObject *object);

bool ut_object_is_file(UtObject *object);