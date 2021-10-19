#include <assert.h>
#include <stdlib.h>

#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-object-array.h"
#include "ut-object-list.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  UtObject **data;
  size_t data_length;
} UtObjectArray;

static UtObject *ut_object_array_get_element(UtObject *object, size_t index) {
  UtObjectArray *self = (UtObjectArray *)object;
  return self->data[index];
}

static UtObjectListFunctions object_list_functions = {
    .get_element = ut_object_array_get_element};

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

static UtMutableListFunctions mutable_list_functions = {
    .insert = ut_object_array_insert,
    .remove = ut_object_array_remove,
    .resize = ut_object_array_resize};

static size_t ut_object_array_get_length(UtObject *object) {
  UtObjectArray *self = (UtObjectArray *)object;
  return self->data_length;
}

static UtObject *ut_object_array_get_element_ref(UtObject *object,
                                                 size_t index) {
  UtObjectArray *self = (UtObjectArray *)object;
  return ut_object_ref(self->data[index]);
}

static UtListFunctions list_functions = {
    .get_length = ut_object_array_get_length,
    .get_element = ut_object_array_get_element_ref};

static void ut_object_array_init(UtObject *object) {
  UtObjectArray *self = (UtObjectArray *)object;
  self->data = NULL;
  self->data_length = 0;
}

static void ut_object_array_cleanup(UtObject *object) {
  UtObjectArray *self = (UtObjectArray *)object;
  for (int i = 0; i < self->data_length; i++) {
    ut_object_unref(self->data[i]);
  }
  free(self->data);
  self->data_length = 0;
}

static UtObjectFunctions object_functions = {
    .type_name = "UtObjectArray",
    .init = ut_object_array_init,
    .to_string = ut_list_to_string,
    .cleanup = ut_object_array_cleanup,
    .interfaces = {{&ut_object_list_id, &object_list_functions},
                   {&ut_mutable_list_id, &mutable_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_object_array_new() {
  return ut_object_new(sizeof(UtObjectArray), &object_functions);
}

void ut_object_array_append(UtObject *object, UtObject *element) {
  assert(ut_object_is_type(object, &object_functions));
  UtObjectArray *self = (UtObjectArray *)object;

  self->data_length++;
  self->data = realloc(self->data, sizeof(UtObject *) * self->data_length);
  self->data[self->data_length - 1] = ut_object_ref(element);
}

bool ut_object_is_object_array(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
