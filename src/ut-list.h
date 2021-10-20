#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  size_t (*get_length)(UtObject *object);
  UtObject *(*get_element)(UtObject *object, size_t index);
} UtListFunctions;

extern int ut_list_id;

UtObject *ut_list_new();

size_t ut_list_get_length(UtObject *object);

// Returns a reference.
UtObject *ut_list_get_element(UtObject *object, size_t index);

char *ut_list_to_string(UtObject *object);

bool ut_object_implements_list(UtObject *object);
