#include <assert.h>

#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-uint16-list.h"
#include "ut-uint32-array.h"
#include "ut-uint32-list.h"
#include "ut-utf16-decoder.h"

typedef struct {
  UtObject object;
  UtObject *input;
  UtObject *buffer;

  UtInputStreamCallback callback;
  void *user_data;
} UtUtf16Decoder;

static void ut_utf16_decoder_init(UtObject *object) {
  UtUtf16Decoder *self = (UtUtf16Decoder *)object;
  self->input = NULL;
  self->buffer = ut_uint32_array_new();
  self->callback = NULL;
  self->user_data = NULL;
}

static void ut_utf16_decoder_cleanup(UtObject *object) {
  UtUtf16Decoder *self = (UtUtf16Decoder *)object;
  ut_object_unref(self->input);
  ut_object_unref(self->buffer);
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtUtf16Decoder *self = user_data;

  size_t utf16_data_length = ut_list_get_length(data);
  size_t offset = 0;
  while (offset < utf16_data_length) {
    uint32_t code_point;
    uint16_t code_unit = ut_uint16_list_get_element(data, offset);
    if (code_unit <= 0xd7ff || code_unit >= 0xe000) {
      code_point = code_unit;
      offset++;
    } else if ((code_unit & 0xfc00) == 0xd800) {
      if (offset + 2 > utf16_data_length) {
        break;
      }
      uint16_t code_unit2 = ut_uint16_list_get_element(data, offset + 1);
      if ((code_unit2 & 0xfc00) == 0xdc00) {
        code_point =
            0x10000 + ((code_unit & 0x3ff) << 10 | (code_unit2 & 0x3ff));
      } else {
        code_point = 0xfffd;
      }
      offset += 2;
    } else {
      code_point = 0xfffd;
      offset++;
    }

    ut_uint32_list_append(self->buffer, code_point);
  }

  size_t n_used = self->callback(self->user_data, self->buffer, complete);
  ut_list_remove(self->buffer, 0, n_used);

  return offset;
}

static void ut_utf16_decoder_read(UtObject *object,
                                  UtInputStreamCallback callback,
                                  void *user_data, UtObject *cancel) {
  UtUtf16Decoder *self = (UtUtf16Decoder *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  ut_input_stream_read(self->input, read_cb, self, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_utf16_decoder_read};

static UtObjectInterface object_interface = {
    .type_name = "UtUtf16Decoder",
    .init = ut_utf16_decoder_init,
    .cleanup = ut_utf16_decoder_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_utf16_decoder_new(UtObject *input) {
  assert(ut_object_implements_input_stream(input));
  UtObject *object = ut_object_new(sizeof(UtUtf16Decoder), &object_interface);
  UtUtf16Decoder *self = (UtUtf16Decoder *)object;
  self->input = ut_object_ref(input);
  return object;
}

bool ut_object_is_utf16_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
