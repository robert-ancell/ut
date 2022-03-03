#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_file_descriptor_new(int fd);

int ut_file_descriptor_get_fd(UtObject *object);

int ut_file_descriptor_take_fd(UtObject *object);

void ut_file_descriptor_close(UtObject *object);

bool ut_object_is_file_descriptor(UtObject *object);
