#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-string-array.h"
#include "ut-string-list.h"
#include "ut-string.h"

typedef struct {
  UtObject object;
  char **data;
  size_t data_length;
} UtStringArray;

static void resize(UtStringArray *self, size_t length) {
  for (size_t i = length; i < self->data_length; i++) {
    free(self->data[i]);
  }
  self->data = realloc(self->data, sizeof(char *) * (length + 1));
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = strdup("");
  }
  self->data[length] = NULL;
  self->data_length = length;
}

static void insert(UtStringArray *self, size_t index, const char *value) {
  self->data_length++;
  self->data = realloc(self->data, sizeof(char *) * (self->data_length + 1));
  for (size_t i = self->data_length - 1; i > index; i--) {
    self->data[i] = self->data[i - 1];
  }
  self->data[index] = strdup(value);
  self->data[self->data_length] = NULL;
}

static size_t ut_string_array_get_length(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  return self->data_length;
}

static UtObject *ut_string_array_get_element_object(UtObject *object,
                                                    size_t index) {
  UtStringArray *self = (UtStringArray *)object;
  return ut_string_new(self->data[index]);
}

static UtObject *ut_string_array_copy(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  UtStringArray *copy = (UtStringArray *)ut_string_array_new();
  copy->data = malloc(sizeof(char *) * (self->data_length + 1));
  copy->data_length = self->data_length;
  for (size_t i = 0; i < self->data_length; i++) {
    copy->data[i] = strdup(self->data[i]);
  }
  copy->data[copy->data_length] = NULL;
  return (UtObject *)copy;
}

static void ut_string_array_insert_object(UtObject *object, size_t index,
                                          UtObject *item) {
  UtStringArray *self = (UtStringArray *)object;
  assert(ut_object_implements_string(item));
  insert(self, index, ut_string_get_text(item));
}

static void ut_string_array_remove(UtObject *object, size_t index,
                                   size_t count) {
  UtStringArray *self = (UtStringArray *)object;
  assert(index <= self->data_length);
  assert(index + count <= self->data_length);
  for (size_t i = index; i <= self->data_length - count; i++) {
    self->data[i] = self->data[i + count];
  }
  resize(self, self->data_length - count);
}

static void ut_string_array_resize(UtObject *object, size_t length) {
  UtStringArray *self = (UtStringArray *)object;
  resize(self, length);
}

static const char *ut_string_array_get_element(UtObject *object, size_t index) {
  UtStringArray *self = (UtStringArray *)object;
  return self->data[index];
}

static char **ut_string_array_take_data(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  char **value = self->data;
  self->data = malloc(sizeof(char *) * 1);
  self->data[0] = NULL;
  self->data_length = 0;
  return value;
}

static void ut_string_array_init(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  self->data = malloc(sizeof(char *) * 1);
  self->data[0] = NULL;
  self->data_length = 0;
}

static void ut_string_array_cleanup(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  free(self->data);
}

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_string_array_get_length,
    .get_element = ut_string_array_get_element_object,
    .copy = ut_string_array_copy,
    .insert = ut_string_array_insert_object,
    .remove = ut_string_array_remove,
    .resize = ut_string_array_resize};

static UtStringListInterface string_list_interface = {
    .get_element = ut_string_array_get_element,
    .take_data = ut_string_array_take_data,
    .insert = ut_string_array_insert};

static UtObjectInterface object_interface = {
    .type_name = "UtStringArray",
    .init = ut_string_array_init,
    .to_string = ut_list_to_string,
    .cleanup = ut_string_array_cleanup,
    .interfaces = {{&ut_list_id, &list_interface},
                   {&ut_string_list_id, &string_list_interface},
                   {NULL, NULL}}};

UtObject *ut_string_array_new() {
  return ut_object_new(sizeof(UtStringArray), &object_interface);
}

UtObject *ut_string_array_new_from_elements(const char *value, ...) {
  va_list ap;
  va_start(ap, value);
  UtObject *object = ut_string_array_new_from_va_elements(value, ap);
  va_end(ap);

  return object;
}

UtObject *ut_string_array_new_from_va_elements(const char *value, va_list ap) {
  UtObject *object = ut_string_array_new();

  ut_string_array_append(object, value);
  while (true) {
    const char *v = va_arg(ap, const char *);
    if (v == NULL) {
      break;
    }
    ut_string_array_append(object, v);
  }

  return object;
}

void ut_string_array_prepend(UtObject *object, const char *value) {
  assert(ut_object_is_string_array(object));
  UtStringArray *self = (UtStringArray *)object;
  insert(self, 0, value);
}

void ut_string_array_append(UtObject *object, const char *value) {
  assert(ut_object_is_string_array(object));
  UtStringArray *self = (UtStringArray *)object;
  insert(self, self->data_length, value);
}

void ut_string_array_insert(UtObject *object, size_t index, const char *value) {
  assert(ut_object_is_string_array(object));
  UtStringArray *self = (UtStringArray *)object;
  insert(self, index, value);
}

bool ut_object_is_string_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
