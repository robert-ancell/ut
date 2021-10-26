#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ut-end-of-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-string-array.h"
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
  self->data = realloc(self->data, sizeof(char *) * length);
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = strdup("");
  }
  self->data_length = length;
}

static void insert(UtStringArray *self, size_t index, const char *value) {
  self->data_length++;
  self->data = realloc(self->data, sizeof(char *) * self->data_length);
  for (size_t i = self->data_length - 1; i > index; i--) {
    self->data[i] = self->data[i - 1];
  }
  self->data[index] = strdup(value);
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
  for (size_t i = index; i < self->data_length - count; i++) {
    self->data[i] = self->data[i + count];
  }
  resize(self, self->data_length - count);
}

static void ut_string_array_resize(UtObject *object, size_t length) {
  UtStringArray *self = (UtStringArray *)object;
  resize(self, length);
}

static size_t ut_string_array_get_length(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  return self->data_length;
}

static UtObject *ut_string_array_get_element(UtObject *object, size_t index) {
  UtStringArray *self = (UtStringArray *)object;
  return ut_string_new(self->data[index]);
}

static UtObject *ut_string_array_copy(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  UtStringArray *copy = (UtStringArray *)ut_string_array_new();
  copy->data = malloc(sizeof(char *) * self->data_length);
  copy->data_length = self->data_length;
  for (size_t i = 0; i < self->data_length; i++) {
    copy->data[i] = strdup(self->data[i]);
  }
  return (UtObject *)copy;
}

static void ut_string_array_read(UtObject *object,
                                 UtInputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtStringArray *self = (UtStringArray *)object;
  size_t n_used = callback(user_data, object);
  UtObjectRef unused_data = NULL;
  if (n_used != self->data_length) {
    unused_data = ut_string_array_new();
    for (size_t i = n_used; i < self->data_length; i++) {
      ut_string_array_append(unused_data, self->data[i]);
    }
  }
  UtObjectRef eos = ut_end_of_stream_new(unused_data);
  callback(user_data, eos);
}

static void ut_string_array_init(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  self->data = NULL;
  self->data_length = 0;
}

static void ut_string_array_cleanup(UtObject *object) {
  UtStringArray *self = (UtStringArray *)object;
  free(self->data);
}

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_string_array_get_length,
    .get_element = ut_string_array_get_element,
    .copy = ut_string_array_copy,
    .insert = ut_string_array_insert_object,
    .remove = ut_string_array_remove,
    .resize = ut_string_array_resize};

static UtInputStreamInterface input_stream_interface = {
    .read = ut_string_array_read, .read_all = ut_string_array_read};

static UtObjectInterface object_interface = {
    .type_name = "UtStringArray",
    .init = ut_string_array_init,
    .to_string = ut_list_to_string,
    .cleanup = ut_string_array_cleanup,
    .interfaces = {{&ut_list_id, &list_interface},
                   {&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_string_array_new() {
  return ut_object_new(sizeof(UtStringArray), &object_interface);
}

UtObject *ut_string_array_new_with_data(size_t length, ...) {
  UtObject *object = ut_string_array_new();
  UtStringArray *self = (UtStringArray *)object;

  self->data = realloc(self->data, sizeof(char *) * length);
  va_list ap;
  va_start(ap, length);
  for (size_t i = 0; i < length; i++) {
    self->data[i] = strdup(va_arg(ap, const char *));
  }
  va_end(ap);

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
