#include <stdbool.h>
#include <stddef.h>

#pragma once

typedef struct _UtObject UtObject;

typedef struct {
  const char *type_name;
  void (*init)(UtObject *object);
  char *(*to_string)(UtObject *object);
  bool (*equal)(UtObject *object, UtObject *other);
  int (*hash)(UtObject *object);
  void (*cleanup)(UtObject *object);

  struct {
    void *interface_id;
    void *interface;
  } interfaces[];
} UtObjectInterface;

struct _UtObject {
  UtObjectInterface *interface;
  int ref_count;
};

UtObject *ut_object_new(size_t object_size, UtObjectInterface *functions);

void *ut_object_get_interface(UtObject *object, void *interface_id);

const char *ut_object_get_type_name(UtObject *object);

char *ut_object_to_string(UtObject *object);

bool ut_object_equal(UtObject *object, UtObject *other);

int ut_object_get_hash(UtObject *object);

// Returns NULL if [object] is NULL.
UtObject *ut_object_ref(UtObject *object);

// Does nothing if [object] is NULL.
void ut_object_unref(UtObject *object);

static inline void ut_object_set(UtObject **object, UtObject *value) {
  ut_object_unref(*object);
  *object = ut_object_ref(value);
}

static inline void ut_object_clear(UtObject **object) {
  ut_object_unref(*object);
  *object = NULL;
}

#define UtObjectRef UtObject *__attribute__((__cleanup__(ut_object_clear)))

bool ut_object_is_type(UtObject *object, UtObjectInterface *functions);
