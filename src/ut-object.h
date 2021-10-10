#include <stddef.h>

#pragma once

typedef struct _UtObject UtObject;

typedef struct {
  void (*init)(UtObject *object);
  void (*cleanup)(UtObject *object);
  void *(*get_interface)(UtObject *object, void *interface_id);
} UtObjectFunctions;

UtObject *ut_object_new(size_t data_size, UtObjectFunctions *functions);

void *ut_object_get_data(UtObject *object);

UtObject *ut_object_ref(UtObject *object);

void ut_object_unref(UtObject *object);

void *ut_object_get_interface(UtObject *object, void *interface_id);
