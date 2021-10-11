#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  void (*clear)(UtObject *object);
} UtMutableListFunctions;

extern int ut_mutable_list_id;

void ut_mutable_list_clear(UtObject *object);
