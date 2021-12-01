#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_shared_memory_array_new(size_t length);

int ut_shared_memory_array_get_fd(UtObject *object);

uint8_t *ut_shared_memory_array_get_data(UtObject *object);

bool ut_object_is_shared_memory_array(UtObject *object);
