#include <assert.h>

#include "ut-end-of-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-object-private.h"
#include "ut-uint32-list.h"
#include "ut-uint8-array.h"
#include "ut-utf8-encoder.h"

typedef struct {
  UtObject object;
  UtObject *input;
  UtObject *buffer;

  UtInputStreamCallback callback;
  void *user_data;
  bool read_all;
} UtUtf8Encoder;

static void ut_utf8_encoder_init(UtObject *object) {
  UtUtf8Encoder *self = (UtUtf8Encoder *)object;
  self->input = NULL;
  self->buffer = ut_uint8_array_new();
  self->callback = NULL;
  self->user_data = NULL;
  self->read_all = false;
}

static void ut_utf8_encoder_cleanup(UtObject *object) {
  UtUtf8Encoder *self = (UtUtf8Encoder *)object;
  ut_object_unref(self->input);
  ut_object_unref(self->buffer);
}

static size_t read_cb(void *user_data, UtObject *data) {
  UtUtf8Encoder *self = user_data;

  if (ut_object_is_end_of_stream(data)) {
    if (self->read_all) {
      self->callback(self->user_data, self->buffer);
    } else {
      UtObjectRef eos = ut_end_of_stream_new(
          ut_list_get_length(self->buffer) > 0 ? self->buffer : NULL);
      self->callback(self->user_data, self->buffer);
    }
    self->callback = NULL;
    self->user_data = NULL;
    return 0;
  }

  const uint32_t *code_points = ut_uint32_list_get_data(data);
  size_t code_points_length = ut_list_get_length(data);
  for (size_t i = 0; i < code_points_length; i++) {
    uint32_t code_point = code_points[i];
    if (code_point <= 0x7f) {
      ut_uint8_array_append(self->buffer, code_point);
    } else if (code_point <= 0x7ff) {
      ut_uint8_array_append(self->buffer, 0xc0 | (code_point >> 6));
      ut_uint8_array_append(self->buffer, 0x80 | (code_point & 0x3f));
    } else if (code_point <= 0xffff) {
      ut_uint8_array_append(self->buffer, 0xe0 | (code_point >> 12));
      ut_uint8_array_append(self->buffer, 0x80 | ((code_point >> 6) & 0x3f));
      ut_uint8_array_append(self->buffer, 0x80 | (code_point & 0x3f));
    } else if (code_point <= 0x10ffff) {
      ut_uint8_array_append(self->buffer, 0xf0 | (code_point >> 18));
      ut_uint8_array_append(self->buffer, 0x80 | ((code_point >> 12) & 0x3f));
      ut_uint8_array_append(self->buffer, 0x80 | ((code_point >> 6) & 0x3f));
      ut_uint8_array_append(self->buffer, 0x80 | (code_point & 0x3f));
    } else {
      assert(false);
    }
  }

  if (!self->read_all) {
    size_t n_used = self->callback(self->user_data, self->buffer);
    ut_mutable_list_remove(self->buffer, 0, n_used);
  }

  return code_points_length;
}

static void ut_utf8_encoder_read(UtObject *object, size_t block_size,
                                 UtInputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtUtf8Encoder *self = (UtUtf8Encoder *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  self->read_all = false;
  ut_input_stream_read(self->input, block_size, read_cb, self, cancel);
}

static void ut_utf8_encoder_read_all(UtObject *object, size_t block_size,
                                     UtInputStreamCallback callback,
                                     void *user_data, UtObject *cancel) {
  UtUtf8Encoder *self = (UtUtf8Encoder *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  self->read_all = true;
  ut_input_stream_read(self->input, block_size, read_cb, self, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_utf8_encoder_read, .read_all = ut_utf8_encoder_read_all};

static UtObjectInterface object_interface = {
    .type_name = "UtUtf8Encoder",
    .init = ut_utf8_encoder_init,
    .cleanup = ut_utf8_encoder_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_utf8_encoder_new(UtObject *input) {
  assert(ut_object_implements_input_stream(input));
  UtObject *object = ut_object_new(sizeof(UtUtf8Encoder), &object_interface);
  UtUtf8Encoder *self = (UtUtf8Encoder *)object;
  self->input = ut_object_ref(input);
  return object;
}

bool ut_object_is_utf8_encoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
