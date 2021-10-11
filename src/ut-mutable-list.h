#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  void (*resize)(UtObject *object, size_t length);
} UtMutableListFunctions;

extern int ut_mutable_list_id;

void ut_mutable_list_clear(UtObject *object);

void ut_mutable_list_resize(UtObject *object, size_t length);
