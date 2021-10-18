#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-constant-string.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  const char *text;
} UtConstantString;

static const char *ut_constant_string_get_text(UtObject *object) {
  UtConstantString *self = (UtConstantString *)object;
  return self->text;
}

static UtStringFunctions string_functions = {.get_text =
                                                 ut_constant_string_get_text};

static const uint8_t *ut_constant_string_get_data(UtObject *object) {
  UtConstantString *self = (UtConstantString *)object;
  return (const uint8_t *)self->text;
}

static UtUint8ListFunctions uint8_list_functions = {
    .get_data = ut_constant_string_get_data};

static size_t ut_constant_string_get_data_length(UtObject *object) {
  UtConstantString *self = (UtConstantString *)object;
  return strlen(self->text);
}

static UtListFunctions list_functions = {
    .get_length = ut_constant_string_get_data_length};

static UtObjectFunctions object_functions = {
    .type_name = "ConstantString",
    .to_string = ut_string_to_string,
    .equal = ut_string_equal,
    .hash = ut_string_hash,
    .interfaces = {{&ut_string_id, &string_functions},
                   {&ut_uint8_list_id, &uint8_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_constant_string_new(const char *text) {
  UtObject *object = ut_object_new(sizeof(UtConstantString), &object_functions);
  UtConstantString *self = (UtConstantString *)object;
  self->text = text;
  return object;
}

bool ut_object_is_constant_string(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
