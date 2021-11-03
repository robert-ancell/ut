#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-constant-uint8-array.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-uint8-subarray.h"
#include "ut-utf8-string.h"

typedef struct {
  UtObject object;
  UtObject *data;
} UtUtf8String;

static ssize_t get_utf8_code_unit_length(uint32_t code_point) {
  if (code_point <= 0x7f) {
    return 1;
  } else if (code_point <= 0x7ff) {
    return 2;
  } else if (code_point <= 0xffff) {
    return 3;
  } else if (code_point <= 0x10ffff) {
    return 4;
  } else {
    return -1;
  }
}

static void write_utf8_code_unit(uint8_t *data, size_t offset,
                                 uint32_t code_point) {
  if (code_point <= 0x7f) {
    data[offset] = code_point;
  } else if (code_point <= 0x7ff) {
    data[offset] = 0xc0 | (code_point >> 6);
    data[offset + 1] = 0x80 | (code_point & 0x3f);
  } else if (code_point <= 0xffff) {
    data[offset] = 0xe0 | (code_point >> 12);
    data[offset + 1] = 0x80 | ((code_point >> 6) & 0x3f);
    data[offset + 2] = 0x80 | (code_point & 0x3f);
  } else if (code_point <= 0x10ffff) {
    data[offset] = 0xf0 | (code_point >> 18);
    data[offset + 1] = 0x80 | ((code_point >> 12) & 0x3f);
    data[offset + 2] = 0x80 | ((code_point >> 6) & 0x3f);
    data[offset + 3] = 0x80 | (code_point & 0x3f);
  } else {
    assert(false);
  }
}

static const char *ut_utf8_string_get_text(UtObject *object) {
  UtUtf8String *self = (UtUtf8String *)object;
  return (const char *)ut_uint8_array_get_data(self->data);
}

static char *ut_utf8_string_take_text(UtObject *object) {
  UtUtf8String *self = (UtUtf8String *)object;
  return (char *)ut_uint8_list_take_data(self->data);
}

static UtObject *ut_utf8_string_get_utf8(UtObject *object) {
  UtUtf8String *self = (UtUtf8String *)object;
  return ut_uint8_subarray_new(self->data, 0,
                               ut_list_get_length(self->data) - 1);
}

static void ut_utf8_string_clear(UtObject *object) {
  assert(ut_object_is_utf8_string(object));
  UtUtf8String *self = (UtUtf8String *)object;
  ut_list_resize(self->data, 1);
  uint8_t *buffer = ut_uint8_array_get_data(self->data);
  buffer[0] = '\0';
}

static void ut_utf8_string_prepend(UtObject *object, const char *text) {
  assert(ut_object_is_utf8_string(object));
  UtUtf8String *self = (UtUtf8String *)object;
  size_t text_length = strlen(text);
  size_t orig_length = ut_list_get_length(self->data);
  ut_list_resize(self->data, ut_list_get_length(self->data) + text_length);
  uint8_t *data = ut_uint8_array_get_data(self->data);
  size_t data_length = ut_list_get_length(self->data);
  for (size_t i = 0; i < orig_length; i++) {
    data[data_length - i - 1] = data[data_length - i - text_length - 1];
  }
  memcpy(data, text, text_length);
}

static void ut_utf8_string_prepend_code_point(UtObject *object,
                                              uint32_t code_point) {
  assert(ut_object_is_utf8_string(object));
  UtUtf8String *self = (UtUtf8String *)object;
  size_t byte_count = get_utf8_code_unit_length(code_point);
  assert(byte_count > 0);
  size_t orig_length = ut_list_get_length(self->data);
  ut_list_resize(self->data, orig_length + byte_count);
  uint8_t *data = ut_uint8_array_get_data(self->data);
  for (size_t i = orig_length + byte_count - 1; i >= byte_count; i--) {
    data[i] = data[i - byte_count];
  }
  write_utf8_code_unit(data, 0, code_point);
}

static void ut_utf8_string_append(UtObject *object, const char *text) {
  assert(ut_object_is_utf8_string(object));
  UtUtf8String *self = (UtUtf8String *)object;
  size_t text_length = strlen(text);
  size_t orig_length = ut_list_get_length(self->data);
  ut_list_resize(self->data, orig_length + text_length);
  memcpy(ut_uint8_array_get_data(self->data) + orig_length - 1, text,
         text_length + 1);
}

static void ut_utf8_string_append_code_point(UtObject *object,
                                             uint32_t code_point) {
  assert(ut_object_is_utf8_string(object));
  UtUtf8String *self = (UtUtf8String *)object;
  size_t byte_count = get_utf8_code_unit_length(code_point);
  assert(byte_count > 0);
  size_t orig_length = ut_list_get_length(self->data);
  ut_list_resize(self->data, orig_length + byte_count);
  uint8_t *data = ut_uint8_array_get_data(self->data);
  write_utf8_code_unit(data, orig_length - 1, code_point);
  data[orig_length + byte_count - 1] = '\0';
}

static void ut_utf8_string_init(UtObject *object) {
  UtUtf8String *self = (UtUtf8String *)object;
  self->data = ut_uint8_array_new();
}

static void ut_utf8_string_cleanup(UtObject *object) {
  UtUtf8String *self = (UtUtf8String *)object;
  ut_object_unref(self->data);
}

static UtStringInterface string_interface = {
    .is_mutable = true,
    .get_text = ut_utf8_string_get_text,
    .take_text = ut_utf8_string_take_text,
    .get_utf8 = ut_utf8_string_get_utf8,
    .clear = ut_utf8_string_clear,
    .prepend = ut_utf8_string_prepend,
    .prepend_code_point = ut_utf8_string_prepend_code_point,
    .append = ut_utf8_string_append,
    .append_code_point = ut_utf8_string_append_code_point};

static UtObjectInterface object_interface = {
    .type_name = "UtUtf8String",
    .init = ut_utf8_string_init,
    .to_string = ut_string_to_string,
    .equal = ut_string_equal,
    .hash = ut_string_hash,
    .cleanup = ut_utf8_string_cleanup,
    .interfaces = {{&ut_string_id, &string_interface}, {NULL, NULL}}};

UtObject *ut_utf8_string_new(const char *text) {
  return ut_utf8_string_new_sized(text, strlen(text));
}

UtObject *ut_utf8_string_new_sized(const char *text, size_t length) {
  UtObject *object = ut_object_new(sizeof(UtUtf8String), &object_interface);
  UtUtf8String *self = (UtUtf8String *)object;
  ut_list_resize(self->data, length + 1);
  uint8_t *buffer = ut_uint8_array_get_data(self->data);
  memcpy(buffer, text, length);
  buffer[length] = '\0';
  return object;
}

UtObject *ut_utf8_string_new_from_data(UtObject *data) {
  assert(data != NULL);
  size_t data_length = ut_list_get_length(data);
  if (ut_object_is_uint8_array(data)) {
    return ut_utf8_string_new_sized((const char *)ut_uint8_array_get_data(data),
                                    data_length);
  } else if (ut_object_is_constant_uint8_array(data)) {
    return ut_utf8_string_new_sized(
        (const char *)ut_constant_uint8_array_get_data(data), data_length);
  } else {
    assert(ut_object_implements_uint8_list(data));
    UtObject *object = ut_string_new("");
    UtUtf8String *self = (UtUtf8String *)object;
    ut_list_resize(self->data, data_length + 1);
    uint8_t *buffer = ut_uint8_array_get_data(self->data);
    for (size_t i = 0; i < data_length; i++) {
      buffer[i] = ut_uint8_list_get_element(data, i);
    }
    buffer[data_length] = '\0';
    return object;
  }
}

bool ut_object_is_utf8_string(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
