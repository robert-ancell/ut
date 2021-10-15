#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  void (*insert)(UtObject *object, size_t index, UtObject *item);
  void (*resize)(UtObject *object, size_t length);
} UtMutableListFunctions;

extern int ut_mutable_list_id;

void ut_mutable_list_append(UtObject *object, UtObject *item);

void ut_mutable_list_prepend(UtObject *object, UtObject *item);

void ut_mutable_list_insert(UtObject *object, size_t index, UtObject *item);

void ut_mutable_list_clear(UtObject *object);

void ut_mutable_list_resize(UtObject *object, size_t length);

bool ut_object_implements_mutable_list(UtObject *object);
