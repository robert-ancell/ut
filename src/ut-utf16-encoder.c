#include <assert.h>

#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-uint16-list.h"
#include "ut-uint32-list.h"
#include "ut-utf16-encoder.h"

typedef struct {
  UtObject object;
  UtObject *input;
  UtObject *buffer;

  UtInputStreamCallback callback;
  void *user_data;
} UtUtf16Encoder;

static void ut_utf16_encoder_init(UtObject *object) {
  UtUtf16Encoder *self = (UtUtf16Encoder *)object;
  self->input = NULL;
  self->buffer = ut_uint16_list_new();
  self->callback = NULL;
  self->user_data = NULL;
}

static void ut_utf16_encoder_cleanup(UtObject *object) {
  UtUtf16Encoder *self = (UtUtf16Encoder *)object;
  ut_object_unref(self->input);
  ut_object_unref(self->buffer);
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtUtf16Encoder *self = user_data;

  size_t code_points_length = ut_list_get_length(data);
  for (size_t i = 0; i < code_points_length; i++) {
    uint32_t code_point = ut_uint32_list_get_element(data, i);
    if (code_point <= 0xd7ff ||
        (code_point >= 0xe000 && code_point <= 0xffff)) {
      ut_uint16_list_append(self->buffer, code_point);
    } else if (code_point >= 0x10000 && code_point <= 0x10ffff) {
      uint32_t c = code_point - 0x10000;
      ut_uint16_list_append(self->buffer, 0xd800 | (c >> 10));
      ut_uint16_list_append(self->buffer, 0xdc00 | (c & 0x3ff));
    } else {
      assert(false);
    }
  }

  size_t n_used = self->callback(self->user_data, self->buffer, complete);
  ut_list_remove(self->buffer, 0, n_used);

  return code_points_length;
}

static void ut_utf16_encoder_read(UtObject *object,
                                  UtInputStreamCallback callback,
                                  void *user_data, UtObject *cancel) {
  UtUtf16Encoder *self = (UtUtf16Encoder *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  ut_input_stream_read(self->input, read_cb, self, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_utf16_encoder_read};

static UtObjectInterface object_interface = {
    .type_name = "UtUtf16Encoder",
    .init = ut_utf16_encoder_init,
    .cleanup = ut_utf16_encoder_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_utf16_encoder_new(UtObject *input) {
  assert(ut_object_implements_input_stream(input));
  UtObject *object = ut_object_new(sizeof(UtUtf16Encoder), &object_interface);
  UtUtf16Encoder *self = (UtUtf16Encoder *)object;
  self->input = ut_object_ref(input);
  return object;
}

bool ut_object_is_utf16_encoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
