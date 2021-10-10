#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  size_t (*get_length)(UtObject *object);
} UtListFunctions;

extern int ut_list_id;

size_t ut_list_get_length(UtObject *object);
