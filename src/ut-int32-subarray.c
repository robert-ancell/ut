#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut-int32-array.h"
#include "ut-int32-list.h"
#include "ut-int32-subarray.h"
#include "ut-int32.h"
#include "ut-list.h"
#include "ut-string.h"

typedef struct {
  UtObject object;
  UtObject *parent;
  size_t parent_length; // Parent length to catch crash if parent changes size.
  size_t start;
  size_t length;
} UtInt32Subarray;

static int32_t *get_data(UtInt32Subarray *self) {
  assert(ut_list_get_length(self->parent) == self->parent_length);
  return ut_int32_array_get_data(self->parent) + self->start;
}

static int32_t ut_int32_subarray_get_element(UtObject *object, size_t index) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  int32_t *data = get_data(self);
  return data[index];
}

static int32_t *ut_int32_subarray_take_data(UtObject *object) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  int32_t *data = get_data(self);
  int32_t *copy = malloc(sizeof(int32_t) * self->length);
  memcpy(copy, data, self->length);
  return copy;
}

static size_t ut_int32_subarray_get_length(UtObject *object) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  return self->length;
}

static UtObject *ut_int32_subarray_get_element_object(UtObject *object,
                                                      size_t index) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  int32_t *data = get_data(self);
  return ut_int32_new(data[index]);
}

static UtObject *ut_int32_subarray_get_sublist(UtObject *object, size_t start,
                                               size_t count) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  return ut_int32_subarray_new(self->parent, self->start + start, count);
}

static UtObject *ut_int32_subarray_copy(UtObject *object) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  UtObject *copy = ut_int32_array_new();
  int32_t *data = get_data(self);
  ut_int32_list_append_block(copy, data, self->length);
  return copy;
}

static void ut_int32_subarray_init(UtObject *object) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  self->parent = NULL;
  self->parent_length = 0;
  self->start = 0;
  self->length = 0;
}

static char *ut_int32_subarray_to_string(UtObject *object) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  int32_t *data = get_data(self);
  UtObjectRef string = ut_string_new("<int32>[");
  for (size_t i = 0; i < self->length; i++) {
    if (i != 0) {
      ut_string_append(string, ", ");
    }
    char value_string[4];
    snprintf(value_string, 4, "%d", data[i]);
    ut_string_append(string, value_string);
  }
  ut_string_append(string, "]");

  return ut_string_take_text(string);
}

static void ut_int32_subarray_cleanup(UtObject *object) {
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  ut_object_unref(self->parent);
}

static UtInt32ListInterface int32_list_interface = {
    .get_element = ut_int32_subarray_get_element,
    .take_data = ut_int32_subarray_take_data};

static UtListInterface list_interface = {
    .is_mutable = false,
    .get_length = ut_int32_subarray_get_length,
    .get_element = ut_int32_subarray_get_element_object,
    .get_sublist = ut_int32_subarray_get_sublist,
    .copy = ut_int32_subarray_copy};

static UtObjectInterface object_interface = {
    .type_name = "UtInt32Subarray",
    .init = ut_int32_subarray_init,
    .to_string = ut_int32_subarray_to_string,
    .cleanup = ut_int32_subarray_cleanup,
    .interfaces = {{&ut_int32_list_id, &int32_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_int32_subarray_new(UtObject *parent, size_t start, size_t length) {
  UtObject *object = ut_object_new(sizeof(UtInt32Subarray), &object_interface);
  UtInt32Subarray *self = (UtInt32Subarray *)object;

  assert(parent != NULL && ut_object_is_int32_array(parent));
  size_t parent_length = ut_list_get_length(parent);
  assert(start + length <= parent_length);

  self->parent = ut_object_ref(parent);
  self->parent_length = parent_length;
  self->start = start;
  self->length = length;
  return object;
}

int32_t *ut_int32_subarray_get_data(UtObject *object) {
  assert(ut_object_is_int32_subarray(object));
  UtInt32Subarray *self = (UtInt32Subarray *)object;
  return get_data(self);
}

bool ut_object_is_int32_subarray(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
