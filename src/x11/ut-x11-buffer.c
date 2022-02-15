#include <assert.h>

#include "ut-file-descriptor.h"
#include "ut-list.h"
#include "ut-uint8-array-with-fds.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-x11-buffer.h"

UtObject *ut_x11_buffer_new() {
  UtObjectRef data = ut_uint8_array_new();
  UtObjectRef fds = ut_list_new();
  return ut_uint8_array_with_fds_new(data, fds);
}

UtObject *ut_x11_buffer_new_with_data(UtObject *data) {
  if (ut_object_is_uint8_array_with_fds(data)) {
    return ut_uint8_array_with_fds_new(ut_uint8_array_with_fds_get_data(data),
                                       ut_uint8_array_with_fds_get_fds(data));
  } else {
    UtObjectRef fds = ut_list_new();
    return ut_uint8_array_with_fds_new(data, fds);
  }
}

UtObject *ut_x11_buffer_get_data(UtObject *object) {
  return ut_uint8_array_with_fds_get_data(object);
}

UtObject *ut_x11_buffer_get_fds(UtObject *object) {
  return ut_uint8_array_with_fds_get_fds(object);
}

void ut_x11_buffer_append_card8(UtObject *object, uint8_t value) {
  ut_uint8_list_append(ut_uint8_array_with_fds_get_data(object), value);
}

void ut_x11_buffer_append_bool(UtObject *object, bool value) {
  ut_x11_buffer_append_card8(object, value ? 1 : 0);
}

void ut_x11_buffer_append_padding(UtObject *object, size_t count) {
  for (size_t i = 0; i < count; i++) {
    ut_x11_buffer_append_card8(object, 0);
  }
}

void ut_x11_buffer_append_align_padding(UtObject *object, size_t alignment) {
  size_t extra = ut_list_get_length(object) % alignment;
  if (extra != 0) {
    ut_x11_buffer_append_padding(object, alignment - extra);
  }
}

void ut_x11_buffer_append_card16(UtObject *object, uint16_t value) {
  ut_uint8_list_append_uint16_le(ut_uint8_array_with_fds_get_data(object),
                                 value);
}

void ut_x11_buffer_append_int16(UtObject *object, int16_t value) {
  ut_x11_buffer_append_card16(object, (uint16_t)value);
}

void ut_x11_buffer_append_card32(UtObject *object, uint32_t value) {
  ut_uint8_list_append_uint32_le(ut_uint8_array_with_fds_get_data(object),
                                 value);
}

void ut_x11_buffer_append_value_card16(UtObject *object, uint16_t value) {
  ut_x11_buffer_append_card32(object, value);
}

void ut_x11_buffer_append_value_int16(UtObject *object, int16_t value) {
  ut_x11_buffer_append_value_card16(object, (uint16_t)value);
}

void ut_x11_buffer_append_block(UtObject *object, const uint8_t *data,
                                size_t data_length) {
  for (size_t i = 0; i < data_length; i++) {
    ut_x11_buffer_append_card8(object, data[i]);
  }
}

void ut_x11_buffer_append_string8(UtObject *object, const char *value) {
  for (const char *c = value; *c != '\0'; c++) {
    ut_x11_buffer_append_card8(object, *c);
  }
}

void ut_x11_buffer_append_fd(UtObject *object, UtObject *fd) {
  ut_list_append(ut_uint8_array_with_fds_get_fds(object), fd);
}

uint8_t ut_x11_buffer_get_card8(UtObject *object, size_t *offset) {
  uint8_t value = ut_uint8_list_get_element(
      ut_uint8_array_with_fds_get_data(object), *offset);
  (*offset)++;
  return value;
}

bool ut_x11_buffer_get_bool(UtObject *object, size_t *offset) {
  return ut_x11_buffer_get_card8(object, offset) != 0;
}

void ut_x11_buffer_get_padding(UtObject *object, size_t *offset, size_t count) {
  (*offset) += count;
}

void ut_x11_buffer_get_align_padding(UtObject *object, size_t *offset,
                                     size_t alignment) {
  while ((*offset) % alignment != 0) {
    (*offset)++;
  }
}

uint16_t ut_x11_buffer_get_card16(UtObject *object, size_t *offset) {
  uint16_t byte1 = ut_x11_buffer_get_card8(object, offset);
  uint16_t byte2 = ut_x11_buffer_get_card8(object, offset);
  return byte1 | byte2 << 8;
}

int16_t ut_x11_buffer_get_int16(UtObject *object, size_t *offset) {
  return (int16_t)ut_x11_buffer_get_card16(object, offset);
}

uint32_t ut_x11_buffer_get_card32(UtObject *object, size_t *offset) {
  uint32_t byte1 = ut_x11_buffer_get_card8(object, offset);
  uint32_t byte2 = ut_x11_buffer_get_card8(object, offset);
  uint32_t byte3 = ut_x11_buffer_get_card8(object, offset);
  uint32_t byte4 = ut_x11_buffer_get_card8(object, offset);
  return byte1 | byte2 << 8 | byte3 << 16 | byte4 << 24;
}

char *ut_x11_buffer_get_string8(UtObject *object, size_t *offset,
                                size_t length) {
  UtObject *data = ut_uint8_array_with_fds_get_data(object);
  assert(ut_list_get_length(data) >= *offset + length);
  UtObjectRef value = ut_uint8_array_new();
  for (size_t i = 0; i < length; i++) {
    ut_uint8_list_append(value, ut_uint8_list_get_element(data, *offset));
    (*offset)++;
  }
  ut_uint8_list_append(value, '\0');
  return (char *)ut_uint8_list_take_data(value);
}

size_t ut_x11_buffer_get_fd_count(UtObject *object) {
  UtObject *fds = ut_uint8_array_with_fds_get_fds(object);
  return ut_list_get_length(fds);
}

UtObject *ut_x11_buffer_take_fd(UtObject *object) {
  UtObject *fds = ut_uint8_array_with_fds_get_fds(object);
  if (ut_list_get_length(fds) == 0) {
    return NULL;
  }
  UtObjectRef fd = ut_list_get_element(fds, 0);
  ut_list_remove(fds, 0, 1);

  return ut_object_ref(fd);
}

bool ut_object_is_x11_buffer(UtObject *object) {
  return ut_object_is_uint8_array(object);
}
