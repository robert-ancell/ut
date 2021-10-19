#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_file_new(const char *path);

void ut_file_open_read(UtObject *object);

void ut_file_open_write(UtObject *object, bool create);

void ut_file_close(UtObject *object);

bool ut_object_is_file(UtObject *object);
