#include <stdbool.h>
#include <stddef.h>

#pragma once

typedef struct _UtObject UtObject;

typedef struct {
  const char *(*get_type_name)();
  void (*init)(UtObject *object);
  void (*cleanup)(UtObject *object);

  struct {
    void *interface_id;
    void *functions;
  } interfaces[];
} UtObjectFunctions;

UtObject *ut_object_new(size_t data_size, UtObjectFunctions *functions);

bool ut_object_is_type(UtObject *object, UtObjectFunctions *functions);

const char *ut_object_get_type_name(UtObject *object);

void *ut_object_get_data(UtObject *object);

UtObject *ut_object_ref(UtObject *object);

void ut_object_unref(UtObject *object);

void *ut_object_get_interface(UtObject *object, void *interface_id);
