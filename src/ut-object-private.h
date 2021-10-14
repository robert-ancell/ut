#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const char *(*get_type_name)();
  void (*init)(UtObject *object);
  char *(*to_string)(UtObject *object);
  void (*cleanup)(UtObject *object);

  struct {
    void *interface_id;
    void *functions;
  } interfaces[];
} UtObjectFunctions;

struct _UtObject {
  UtObjectFunctions *functions;
  int ref_count;
};

UtObject *ut_object_new(size_t object_size, UtObjectFunctions *functions);

bool ut_object_is_type(UtObject *object, UtObjectFunctions *functions);

void *ut_object_get_interface(UtObject *object, void *interface_id);
