#include <assert.h>

#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-object-private.h"
#include "ut-uint32-array.h"
#include "ut-uint8-list.h"
#include "ut-utf8-decoder.h"

typedef struct {
  UtObject object;
  UtObject *input;
  UtObject *buffer;

  UtInputStreamCallback callback;
  void *user_data;
  bool read_all;
} UtUtf8Decoder;

static void ut_utf8_decoder_init(UtObject *object) {
  UtUtf8Decoder *self = (UtUtf8Decoder *)object;
  self->input = NULL;
  self->buffer = ut_uint32_array_new();
  self->callback = NULL;
  self->user_data = NULL;
  self->read_all = false;
}

static void ut_utf8_decoder_cleanup(UtObject *object) {
  UtUtf8Decoder *self = (UtUtf8Decoder *)object;
  ut_object_unref(self->input);
  ut_object_unref(self->buffer);
}

static size_t read_cb(void *user_data, UtObject *buffer) {
  UtUtf8Decoder *self = user_data;

  const uint8_t *data = ut_uint8_list_get_data(buffer);
  size_t data_length = ut_list_get_length(buffer);
  size_t offset = 0;
  while (offset < data_length) {
    uint32_t code_point;
    uint8_t byte1 = data[offset];
    if ((byte1 & 0x80) == 0) {
      code_point = byte1;
      offset++;
    } else if ((byte1 & 0xe0) == 0xc0) {
      if (offset + 2 > data_length) {
        break;
      }
      uint8_t byte2 = data[offset + 1];
      if ((byte2 & 0xc0) != 0x80) {
        code_point = 0xfffd;
      }
      code_point = (byte1 & 0x1f) << 6 | (byte2 & 0x3f);
      offset += 2;
      return true;
    } else if ((byte1 & 0xf0) == 0xe0) {
      if (offset + 3 > data_length) {
        break;
      }
      uint8_t byte2 = data[offset + 1], byte3 = data[offset + 2];
      if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80) {
        code_point = 0xfffd;
      } else {
        code_point =
            (byte1 & 0x0f) << 12 | (byte2 & 0x3f) << 6 | (byte3 & 0x3f);
      }
      offset += 3;
      return true;
    } else if ((byte1 & 0xf8) == 0xf0) {
      if (offset + 4 > data_length) {
        break;
      }
      uint8_t byte2 = data[offset + 1], byte3 = data[offset + 2],
              byte4 = data[offset + 3];
      if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80 ||
          (byte4 & 0xc0) != 0x80) {
        code_point = 0xfffd;
      } else {
        code_point = (byte1 & 0x07) << 18 | (byte2 & 0x3f) << 12 |
                     (byte3 & 0x3f) << 6 | (byte4 & 0x3f);
      }
      offset += 4;
    } else {
      code_point = 0xfffd;
      offset++;
    }

    ut_uint32_array_append(self->buffer, code_point);
  }

  // FIXME: Need better way to know if this is EOS, as we will always try and
  // reprocess the final data.
  bool is_eos = ut_list_get_length(buffer) == 0;
  if (!self->read_all || is_eos) {
    if (ut_list_get_length(self->buffer) > 0) {
      size_t n_used = self->callback(self->user_data, self->buffer);
      ut_mutable_list_remove(self->buffer, 0, n_used);
    }

    if (is_eos) {
      assert(ut_list_get_length(self->buffer) == 0);
      self->callback(self->user_data, self->buffer);
    }
  }

  return offset;
}

static void ut_utf8_decoder_read(UtObject *object, size_t block_size,
                                 UtInputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtUtf8Decoder *self = (UtUtf8Decoder *)object;
  assert(callback != NULL);
  self->callback = callback;
  self->user_data = user_data;
  self->read_all = false;
  ut_input_stream_read(self->input, block_size, read_cb, self, cancel);
}

static void ut_utf8_decoder_read_all(UtObject *object, size_t block_size,
                                     UtInputStreamCallback callback,
                                     void *user_data, UtObject *cancel) {
  UtUtf8Decoder *self = (UtUtf8Decoder *)object;
  assert(callback != NULL);
  self->callback = callback;
  self->user_data = user_data;
  self->read_all = true;
  ut_input_stream_read(self->input, block_size, read_cb, self, cancel);
}

static UtInputStreamFunctions input_stream_functions = {
    .read = ut_utf8_decoder_read, .read_all = ut_utf8_decoder_read_all};

static UtObjectFunctions object_functions = {
    .type_name = "UtUtf8Decoder",
    .init = ut_utf8_decoder_init,
    .cleanup = ut_utf8_decoder_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_functions},
                   {NULL, NULL}}};

UtObject *ut_utf8_decoder_new(UtObject *input) {
  assert(ut_object_implements_input_stream(input));
  UtObject *object = ut_object_new(sizeof(UtUtf8Decoder), &object_functions);
  UtUtf8Decoder *self = (UtUtf8Decoder *)object;
  self->input = ut_object_ref(input);
  return object;
}

bool ut_object_is_utf8_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
