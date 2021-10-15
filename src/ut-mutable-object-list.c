#include <assert.h>
#include <stdlib.h>

#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-mutable-object-list.h"
#include "ut-object-list.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  UtObject **data;
  size_t data_length;
} UtMutableObjectList;

UtObject *ut_mutable_object_list_get_element(UtObject *object, size_t index) {
  UtMutableObjectList *self = (UtMutableObjectList *)object;
  return self->data[index];
}

static UtObjectListFunctions object_list_functions = {
    .get_element = ut_mutable_object_list_get_element};

void ut_mutable_object_list_resize(UtObject *object, size_t length) {
  UtMutableObjectList *self = (UtMutableObjectList *)object;
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
    .resize = ut_mutable_object_list_resize};

static size_t ut_mutable_object_list_get_length(UtObject *object) {
  UtMutableObjectList *self = (UtMutableObjectList *)object;
  return self->data_length;
}

static UtObject *ut_mutable_object_list_get_element_ref(UtObject *object,
                                                        size_t index) {
  UtMutableObjectList *self = (UtMutableObjectList *)object;
  return ut_object_ref(self->data[index]);
}

static UtListFunctions list_functions = {
    .get_length = ut_mutable_object_list_get_length,
    .get_element = ut_mutable_object_list_get_element_ref};

static void ut_mutable_object_list_init(UtObject *object) {
  UtMutableObjectList *self = (UtMutableObjectList *)object;
  self->data = NULL;
  self->data_length = 0;
}

static void ut_mutable_object_list_cleanup(UtObject *object) {
  UtMutableObjectList *self = (UtMutableObjectList *)object;
  for (int i = 0; i < self->data_length; i++) {
    ut_object_unref(self->data[i]);
  }
  free(self->data);
  self->data_length = 0;
}

static UtObjectFunctions object_functions = {
    .type_name = "MutableObjectList",
    .init = ut_mutable_object_list_init,
    .cleanup = ut_mutable_object_list_cleanup,
    .interfaces = {{&ut_object_list_id, &object_list_functions},
                   {&ut_mutable_list_id, &mutable_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_mutable_object_list_new() {
  return ut_object_new(sizeof(UtMutableObjectList), &object_functions);
}

void ut_mutable_object_list_append(UtObject *object, UtObject *element) {
  assert(ut_object_is_type(object, &object_functions));
  UtMutableObjectList *self = (UtMutableObjectList *)object;

  self->data_length++;
  self->data = realloc(self->data, sizeof(UtObject *) * self->data_length);
  self->data[self->data_length - 1] = ut_object_ref(element);
}

bool ut_object_is_mutable_object_list(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
