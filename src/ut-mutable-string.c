#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-mutable-string.h"
#include "ut-mutable-uint8-list.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  UtObject *data;
} UtMutableString;

static const char *ut_mutable_string_get_text(UtObject *object) {
  UtMutableString *self = (UtMutableString *)object;
  return (const char *)ut_uint8_list_get_data(self->data);
}

static UtStringFunctions string_functions = {.get_text =
                                                 ut_mutable_string_get_text};

static const uint8_t *ut_mutable_string_get_data(UtObject *object) {
  UtMutableString *self = (UtMutableString *)object;
  return ut_uint8_list_get_data(self->data);
}

static UtUint8ListFunctions uint8_list_functions = {
    .get_data = ut_mutable_string_get_data};

static size_t ut_mutable_string_get_length(UtObject *object) {
  UtMutableString *self = (UtMutableString *)object;
  return ut_list_get_length(self->data);
}

static UtListFunctions list_functions = {.get_length =
                                             ut_mutable_string_get_length};

static void ut_mutable_string_init(UtObject *object) {
  UtMutableString *self = (UtMutableString *)object;
  self->data = ut_mutable_uint8_list_new();
}

static void ut_mutable_string_cleanup(UtObject *object) {
  UtMutableString *self = (UtMutableString *)object;
  ut_object_unref(self->data);
}

static UtObjectFunctions object_functions = {
    .type_name = "MutableString",
    .init = ut_mutable_string_init,
    .cleanup = ut_mutable_string_cleanup,
    .interfaces = {{&ut_string_id, &string_functions},
                   {&ut_uint8_list_id, &uint8_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_mutable_string_new(const char *text) {
  UtObject *object = ut_object_new(sizeof(UtMutableString), &object_functions);
  UtMutableString *self = (UtMutableString *)object;
  ut_mutable_list_resize(self->data, strlen(text));
  memcpy(ut_mutable_uint8_list_get_data(self->data), text, strlen(text) + 1);
  return object;
}

void ut_mutable_string_clear(UtObject *object) {
  UtMutableString *self = (UtMutableString *)object;
  ut_mutable_list_clear(self->data);
  ut_mutable_uint8_list_append(self->data, '\0');
}

void ut_mutable_string_prepend(UtObject *object, const char *text) {
  UtMutableString *self = (UtMutableString *)object;
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

void ut_mutable_string_append(UtObject *object, const char *text) {
  UtMutableString *self = (UtMutableString *)object;
  size_t text_length = strlen(text);
  size_t orig_length = ut_list_get_length(self->data);
  ut_mutable_list_resize(self->data, orig_length + text_length);
  memcpy(ut_mutable_uint8_list_get_data(self->data) + orig_length, text,
         text_length + 1);
}

bool ut_object_is_mutable_string(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
