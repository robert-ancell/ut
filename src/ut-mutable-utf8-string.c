#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-mutable-string.h"
#include "ut-mutable-uint8-list.h"
#include "ut-mutable-utf8-string.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint8-list.h"
#include "ut-utf8-string.h"

typedef struct {
  UtObject *data;
} UtMutableUtf8String;

static const char *ut_mutable_utf8_string_get_text(UtObject *object) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  return (const char *)ut_uint8_list_get_data(self->data);
}

static UtUtf8StringFunctions utf8_string_functions = {
    .get_text = ut_mutable_utf8_string_get_text};

static UtObject *ut_mutable_utf8_string_get_code_points(UtObject *object) {
  return ut_utf8_string_get_code_points(object);
}

static UtStringFunctions string_functions = {
    .get_code_points = ut_mutable_utf8_string_get_code_points};

static void ut_mutable_utf8_string_clear(UtObject *object) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  ut_mutable_list_clear(self->data);
  ut_mutable_uint8_list_append(self->data, '\0');
}

static void ut_mutable_utf8_string_prepend(UtObject *object, const char *text) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  size_t text_length = strlen(text);
  size_t orig_length = ut_list_get_length(self->data);
  ut_mutable_list_resize(self->data,
                         ut_list_get_length(self->data) + text_length);
  uint8_t *data = ut_mutable_uint8_list_get_data(self->data);
  size_t data_length = ut_list_get_length(self->data);
  for (size_t i = 0; i < orig_length; i++) {
    data[data_length - i - 1] = data[data_length - i - text_length - 1];
  }
  memcpy(data, text, text_length);
}

static void ut_mutable_utf8_string_append(UtObject *object, const char *text) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  size_t text_length = strlen(text);
  size_t orig_length = ut_list_get_length(self->data);
  ut_mutable_list_resize(self->data, orig_length + text_length);
  memcpy(ut_mutable_uint8_list_get_data(self->data) + orig_length, text,
         text_length + 1);
}

static UtMutableStringFunctions mutable_string_functions = {
    .clear = ut_mutable_utf8_string_clear,
    .prepend = ut_mutable_utf8_string_prepend,
    .append = ut_mutable_utf8_string_append};

static const uint8_t *ut_mutable_utf8_string_get_data(UtObject *object) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  return ut_uint8_list_get_data(self->data);
}

static UtUint8ListFunctions uint8_list_functions = {
    .get_data = ut_mutable_utf8_string_get_data};

static size_t ut_mutable_utf8_string_get_data_length(UtObject *object) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  return ut_list_get_length(self->data);
}

static UtListFunctions list_functions = {
    .get_length = ut_mutable_utf8_string_get_data_length};

static const char *ut_mutable_utf8_string_get_type_name() {
  return "MutableUtf8String";
}

static void ut_mutable_utf8_string_init(UtObject *object) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  self->data = ut_mutable_uint8_list_new();
}

static void ut_mutable_utf8_string_cleanup(UtObject *object) {
  UtMutableUtf8String *self = ut_object_get_data(object);
  ut_object_unref(self->data);
}

static UtObjectFunctions object_functions = {
    .get_type_name = ut_mutable_utf8_string_get_type_name,
    .init = ut_mutable_utf8_string_init,
    .cleanup = ut_mutable_utf8_string_cleanup,
    .interfaces = {{&ut_utf8_string_id, &utf8_string_functions},
                   {&ut_string_id, &string_functions},
                   {&ut_mutable_string_id, &mutable_string_functions},
                   {&ut_uint8_list_id, &uint8_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_mutable_utf8_string_new(const char *text) {
  UtObject *object =
      ut_object_new(sizeof(UtMutableUtf8String), &object_functions);
  UtMutableUtf8String *self = ut_object_get_data(object);
  ut_mutable_list_resize(self->data, strlen(text));
  memcpy(ut_mutable_uint8_list_get_data(self->data), text, strlen(text) + 1);
  return object;
}

bool ut_object_is_mutable_utf8_string(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
