#include <assert.h>
#include <stdint.h>

#include "deflate/ut-deflate-decoder.h"
#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-gzip-decoder.h"
#include "ut-gzip-error.h"
#include "ut-input-stream-multiplexer.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-uint8-list.h"

typedef enum {
  DECODER_STATE_MEMBER_HEADER,
  DECODER_STATE_MEMBER_DATA,
  DECODER_STATE_MEMBER_TRAILER,
  DECODER_STATE_DONE,
  DECODER_STATE_ERROR
} DecoderState;

typedef struct {
  UtObject object;
  UtObject *multiplexer;
  UtObject *gzip_input_stream;
  UtObject *deflate_input_stream;
  UtObject *read_cancel;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
  DecoderState state;
  UtObject *deflate_decoder;
  UtObject *buffer;
  UtObject *error;
} UtGzipDecoder;

static size_t deflate_read_cb(void *user_data, UtObject *data, bool complete) {
  UtGzipDecoder *self = user_data;

  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->read_cancel);
    return 0;
  }

  size_t total_used = 0;
  size_t data_length = ut_list_get_length(data);

  do {
    UtObjectRef d =
        total_used == 0
            ? ut_object_ref(data)
            : ut_list_get_sublist(data, total_used, data_length - total_used);
    size_t n_used = self->callback(self->user_data, data, complete);
    total_used += n_used;
  } while (!ut_cancel_is_active(self->cancel) && total_used < data_length);

  assert(total_used <= data_length);

  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->read_cancel);
    return 0;
  }

  if (complete) {
    self->state = DECODER_STATE_MEMBER_TRAILER;
    ut_input_stream_multiplexer_set_active(self->multiplexer,
                                           self->gzip_input_stream);
  }

  return total_used;
}

static char *read_string(UtObject *data, size_t *offset) {
  size_t data_length = ut_list_get_length(data);
  UtObjectRef value = ut_uint8_list_new();
  for (size_t i = *offset; i < data_length; i++) {
    uint8_t c = ut_uint8_list_get_element(data, i);
    ut_uint8_list_append(value, c);
    if (c == 0) {
      return (char *)ut_uint8_list_take_data(value);
    }
  }

  return NULL;
}

static bool read_member_header(UtGzipDecoder *self, UtObject *data,
                               size_t *offset, bool complete) {
  size_t data_length = ut_list_get_length(data);

  if (*offset == data_length && complete) {
    self->state = DECODER_STATE_DONE;
    return true;
  }

  size_t header_start = *offset;
  size_t header_end = header_start + 10;
  if (data_length < header_end) {
    return false;
  }

  uint8_t id1 = ut_uint8_list_get_element(data, header_start + 0);
  uint8_t id2 = ut_uint8_list_get_element(data, header_start + 1);
  if (id1 != 31 || id2 != 139) {
    self->error = ut_gzip_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
  }

  uint8_t compression_method =
      ut_uint8_list_get_element(data, header_start + 2);
  if (compression_method != 8) {
    self->error = ut_gzip_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
  }

  uint8_t flags = ut_uint8_list_get_element(data, header_start + 3);

  if ((flags & 0x04) != 0) {
    header_end += 2;
    if (data_length < header_end) {
      return false;
    }
    uint16_t xlen = ut_uint8_list_get_uint16_le(data, header_start + 10);
    header_end += xlen;
    if (data_length < header_end) {
      return false;
    }
  }

  if ((flags & 0x08) != 0) {
    ut_cstring_ref file_name = read_string(data, &header_end);
    if (file_name == NULL) {
      return false;
    }
  }

  if ((flags & 0x10) != 0) {
    ut_cstring_ref file_comment = read_string(data, &header_end);
    if (file_comment == NULL) {
      return false;
    }
  }

  if ((flags & 0x02) != 0) {
    if (data_length < header_end + 2) {
      return false;
    }
    /*uint16_t crc =*/ut_uint8_list_get_uint16_le(data, header_end);
    header_end += 2;
  }

  *offset = header_end;
  self->state = DECODER_STATE_MEMBER_DATA;
  ut_object_unref(self->deflate_decoder);
  self->deflate_decoder = ut_deflate_decoder_new(self->deflate_input_stream);
  ut_input_stream_multiplexer_set_active(self->multiplexer,
                                         self->deflate_input_stream);
  ut_input_stream_read(self->deflate_decoder, deflate_read_cb, self,
                       self->read_cancel);
  return true;
}

static bool read_member_trailer(UtGzipDecoder *self, UtObject *data,
                                size_t *offset, bool complete) {
  size_t data_length = ut_list_get_length(data);
  size_t trailer_start = *offset;
  size_t trailer_end = trailer_start + 8;
  if (data_length < trailer_end) {
    return false;
  }

  *offset += 8;

  if (complete && data_length == trailer_end) {
    self->state = DECODER_STATE_DONE;
  } else {
    self->state = DECODER_STATE_MEMBER_HEADER;
  }
  return true;
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtGzipDecoder *self = user_data;

  size_t offset = 0;
  bool decoding = true;
  while (decoding) {
    if (ut_cancel_is_active(self->cancel)) {
      ut_cancel_activate(self->read_cancel);
      break;
    }

    switch (self->state) {
    case DECODER_STATE_MEMBER_HEADER:
      decoding = read_member_header(self, data, &offset, complete);
      break;
    case DECODER_STATE_MEMBER_DATA:
      // Will be processed in other stream.
      return offset;
    case DECODER_STATE_MEMBER_TRAILER:
      decoding = read_member_trailer(self, data, &offset, complete);
      break;
    case DECODER_STATE_DONE:
      ut_cancel_activate(self->read_cancel);
      decoding = false;
      break;
    case DECODER_STATE_ERROR:
      ut_cancel_activate(self->read_cancel);
      decoding = false;
      break;
    }
  }

  return offset;
}

static void ut_gzip_decoder_init(UtObject *object) {
  UtGzipDecoder *self = (UtGzipDecoder *)object;
  self->multiplexer = NULL;
  self->gzip_input_stream = NULL;
  self->deflate_input_stream = NULL;
  self->read_cancel = ut_cancel_new();
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
  self->state = DECODER_STATE_MEMBER_HEADER;
  self->deflate_decoder = NULL;
  self->buffer = ut_uint8_list_new();
  self->error = NULL;
}

static void ut_gzip_decoder_cleanup(UtObject *object) {
  UtGzipDecoder *self = (UtGzipDecoder *)object;
  ut_cancel_activate(self->read_cancel);
  ut_object_unref(self->multiplexer);
  ut_object_unref(self->gzip_input_stream);
  ut_object_unref(self->deflate_input_stream);
  ut_object_unref(self->read_cancel);
  ut_object_unref(self->cancel);
  ut_object_unref(self->deflate_decoder);
  ut_object_unref(self->buffer);
  ut_object_unref(self->error);
}

static void ut_gzip_decoder_read(UtObject *object,
                                 UtInputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtGzipDecoder *self = (UtGzipDecoder *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);
  ut_input_stream_multiplexer_set_active(self->multiplexer,
                                         self->gzip_input_stream);
  ut_input_stream_read(self->gzip_input_stream, read_cb, self,
                       self->read_cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_gzip_decoder_read};

static UtObjectInterface object_interface = {
    .type_name = "UtGzipDecoder",
    .init = ut_gzip_decoder_init,
    .cleanup = ut_gzip_decoder_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_gzip_decoder_new(UtObject *input_stream) {
  assert(input_stream != NULL);
  UtObject *object = ut_object_new(sizeof(UtGzipDecoder), &object_interface);
  UtGzipDecoder *self = (UtGzipDecoder *)object;
  self->multiplexer = ut_input_stream_multiplexer_new(input_stream);
  self->gzip_input_stream = ut_input_stream_multiplexer_add(self->multiplexer);
  self->deflate_input_stream =
      ut_input_stream_multiplexer_add(self->multiplexer);
  return object;
}

bool ut_object_is_gzip_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
