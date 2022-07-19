#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ut-list.h"
#include "ut-object-array.h"
#include "ut-object-list.h"

typedef struct {
  UtObject object;
  UtObject **data;
  size_t data_length;
} UtObjectArray;

static UtObject *ut_object_array_get_element(UtObject *object, size_t index) {
  UtObjectArray *self = (UtObjectArray *)object;
  return self->data[index];
}

static void ut_object_array_insert(UtObject *object, size_t index,
                                   UtObject *item) {
  UtObjectArray *self = (UtObjectArray *)object;
  assert(index <= self->data_length);
  self->data_length++;
  self->data = realloc(self->data, sizeof(UtObject *) * self->data_length);
  for (size_t i = self->data_length - 1; i > index; i--) {
    self->data[i] = self->data[i - 1];
  }
  self->data[index] = ut_object_ref(item);
}

static void ut_object_array_remove(UtObject *object, size_t index,
                                   size_t count) {
  UtObjectArray *self = (UtObjectArray *)object;
  assert(index <= self->data_length);
  assert(index + count <= self->data_length);
  for (size_t i = index; i < index + count; i++) {
    ut_object_unref(self->data[i]);
  }
  for (size_t i = index; i < self->data_length - count; i++) {
    self->data[i] = self->data[i + count];
  }
  self->data_length -= count;
  self->data = realloc(self->data, sizeof(UtObject *) * self->data_length);
}

static void ut_object_array_resize(UtObject *object, size_t length) {
  UtObjectArray *self = (UtObjectArray *)object;
  for (size_t i = length; i < self->data_length; i++) {
    ut_object_unref(self->data[i]);
  }
  self->data = realloc(self->data, sizeof(UtObject *) * length);
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = NULL;
  }
  self->data_length = length;
}

static size_t ut_object_array_get_length(UtObject *object) {
  UtObjectArray *self = (UtObjectArray *)object;
  return self->data_length;
}

static UtObject *ut_object_array_get_element_ref(UtObject *object,
                                                 size_t index) {
  UtObjectArray *self = (UtObjectArray *)object;
  return ut_object_ref(self->data[index]);
}

static UtObject *ut_object_array_copy(UtObject *object) {
  UtObjectArray *self = (UtObjectArray *)object;
  UtObjectArray *copy = (UtObjectArray *)ut_object_array_new();
  copy->data = malloc(sizeof(UtObject *) * self->data_length);
  copy->data_length = self->data_length;
  for (size_t i = 0; i < self->data_length; i++) {
    copy->data[i] = ut_object_ref(self->data[i]);
  }
  return (UtObject *)copy;
}

static void ut_object_array_init(UtObject *object) {
  UtObjectArray *self = (UtObjectArray *)object;
  self->data = NULL;
  self->data_length = 0;
}

static void ut_object_array_cleanup(UtObject *object) {
  UtObjectArray *self = (UtObjectArray *)object;
  for (size_t i = 0; i < self->data_length; i++) {
    ut_object_unref(self->data[i]);
  }
  free(self->data);
  self->data_length = 0;
}

static UtObjectListInterface object_list_interface = {
    .get_element = ut_object_array_get_element};

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_object_array_get_length,
    .get_element = ut_object_array_get_element_ref,
    .copy = ut_object_array_copy,
    .insert = ut_object_array_insert,
    .remove = ut_object_array_remove,
    .resize = ut_object_array_resize};

static UtObjectInterface object_interface = {
    .type_name = "UtObjectArray",
    .init = ut_object_array_init,
    .to_string = ut_list_to_string,
    .cleanup = ut_object_array_cleanup,
    .interfaces = {{&ut_object_list_id, &object_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_object_array_new() {
  return ut_object_new(sizeof(UtObjectArray), &object_interface);
}

UtObject *ut_object_array_new_from_elements(UtObject *item0, ...) {
  UtObject *object = ut_object_array_new();
  if (item0 == NULL) {
    return object;
  }

  UtObjectArray *self = (UtObjectArray *)object;

  va_list ap;
  va_list ap2;
  va_copy(ap2, ap);

  size_t length = 1;
  va_start(ap2, item0);
  while (va_arg(ap, UtObject *) != NULL) {
    length++;
  }
  va_end(ap);

  self->data = realloc(self->data, sizeof(UtObject *) * length);
  va_start(ap, item0);
  self->data[0] = ut_object_ref(item0);
  for (size_t i = 1; i < length; i++) {
    self->data[i] = ut_object_ref(va_arg(ap, UtObject *));
  }
  va_end(ap);

  return object;
}

UtObject *ut_object_array_new_from_elements_take(UtObject *item0, ...) {
  UtObject *object = ut_object_array_new();
  if (item0 == NULL) {
    return object;
  }

  UtObjectArray *self = (UtObjectArray *)object;

  va_list ap;
  va_list ap2;
  va_copy(ap2, ap);

  size_t length = 1;
  va_start(ap2, item0);
  while (va_arg(ap, UtObject *) != NULL) {
    length++;
  }
  va_end(ap);

  self->data = realloc(self->data, sizeof(UtObject *) * length);
  va_start(ap, item0);
  self->data[0] = item0;
  for (size_t i = 1; i < length; i++) {
    self->data[i] = va_arg(ap, UtObject *);
  }
  va_end(ap);

  return object;
}

bool ut_object_is_object_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
