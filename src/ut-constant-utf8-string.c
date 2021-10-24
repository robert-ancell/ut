#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-constant-uint8-array.h"
#include "ut-constant-utf8-string.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  const char *text;
} UtConstantUtf8String;

static const char *ut_constant_utf8_string_get_text(UtObject *object) {
  UtConstantUtf8String *self = (UtConstantUtf8String *)object;
  return self->text;
}

static UtObject *ut_constant_utf8_string_get_utf8(UtObject *object) {
  UtConstantUtf8String *self = (UtConstantUtf8String *)object;
  return ut_constant_uint8_array_new((const uint8_t *)self->text,
                                     strlen(self->text) + 1);
}

static UtStringInterface string_interface = {
    .is_mutable = false,
    .get_text = ut_constant_utf8_string_get_text,
    .get_utf8 = ut_constant_utf8_string_get_utf8};

static UtObjectInterface object_interface = {
    .type_name = "UtConstantUtf8String",
    .to_string = ut_string_to_string,
    .equal = ut_string_equal,
    .hash = ut_string_hash,
    .interfaces = {{&ut_string_id, &string_interface}, {NULL, NULL}}};

UtObject *ut_constant_utf8_string_new(const char *text) {
  UtObject *object =
      ut_object_new(sizeof(UtConstantUtf8String), &object_interface);
  UtConstantUtf8String *self = (UtConstantUtf8String *)object;
  self->text = text;
  return object;
}

bool ut_object_is_constant_utf8_string(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
