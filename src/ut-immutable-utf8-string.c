#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-immutable-utf8-string.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint8-list.h"
#include "ut-utf8-string.h"

typedef struct {
  const char *text;
} UtImmutableUtf8String;

static const char *ut_immutable_utf8_string_get_text(UtObject *object) {
  UtImmutableUtf8String *self = ut_object_get_data(object);
  return self->text;
}

static UtUtf8StringFunctions utf8_string_functions = {
    .get_text = ut_immutable_utf8_string_get_text};

static UtObject *ut_immutable_utf8_string_get_code_points(UtObject *object) {
  return ut_utf8_string_get_code_points(object);
}

static UtStringFunctions string_functions = {
    .get_code_points = ut_immutable_utf8_string_get_code_points};

static const uint8_t *ut_immutable_utf8_string_get_data(UtObject *object) {
  UtImmutableUtf8String *self = ut_object_get_data(object);
  return (const uint8_t *)self->text;
}

static UtUint8ListFunctions uint8_list_functions = {
    .get_data = ut_immutable_utf8_string_get_data};

static size_t ut_immutable_utf8_string_get_data_length(UtObject *object) {
  UtImmutableUtf8String *self = ut_object_get_data(object);
  return strlen(self->text) + 1;
}

static UtListFunctions list_functions = {
    .get_length = ut_immutable_utf8_string_get_data_length};

static const char *ut_immutable_utf8_string_get_type_name() {
  return "ImmutableUtf8String";
}

static UtObjectFunctions object_functions = {
    .get_type_name = ut_immutable_utf8_string_get_type_name,
    .interfaces = {{&ut_utf8_string_id, &utf8_string_functions},
                   {&ut_string_id, &string_functions},
                   {&ut_uint8_list_id, &uint8_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_immutable_utf8_string_new(const char *text) {
  UtObject *object =
      ut_object_new(sizeof(UtImmutableUtf8String), &object_functions);
  UtImmutableUtf8String *self = ut_object_get_data(object);
  self->text = text;
  return object;
}

bool ut_object_is_immutable_utf8_string(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
