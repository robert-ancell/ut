#include <assert.h>

#include "ut-cancel.h"
#include "ut-deflate-decoder.h"
#include "ut-deflate-error.h"
#include "ut-huffman-decoder.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef enum {
  DECODER_STATE_BLOCK_HEADER,
  DECODER_STATE_UNCOMPRESSED_LENGTH,
  DECODER_STATE_UNCOMPRESSED_DATA,
  DECODER_STATE_LITERAL_OR_LENGTH,
  DECODER_STATE_LENGTH,
  DECODER_STATE_DISTANCE,
  DECODER_STATE_DONE
} DecoderState;

typedef struct {
  UtObject object;
  UtObject *input_stream;
  UtObject *read_cancel;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;

  uint8_t bit_buffer;
  uint8_t bit_count;

  DecoderState state;
  bool is_last_block;
  uint16_t length;
  uint8_t extra_length_bits;

  UtObject *literal_or_length_decoder;
  UtObject *distance_decoder;

  UtObject *buffer;
} UtDeflateDecoder;

static uint16_t base_lengths[29] = {3,  4,  5,  6,   7,   8,   9,   10,  11, 13,
                                    15, 17, 19, 23,  27,  31,  35,  43,  51, 59,
                                    67, 83, 99, 115, 131, 163, 195, 227, 258};

static uint8_t extra_length_bits[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                                        1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                        4, 4, 4, 4, 5, 5, 5, 5, 0};

static uint16_t base_distances[30] = {
    1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
    33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

static uint8_t distance_bits[29] = {0, 0,  0,  0,  1,  1,  2,  2,  3, 4,
                                    4, 5,  5,  6,  6,  7,  7,  8,  8, 9,
                                    9, 10, 10, 11, 11, 12, 12, 13, 13};

static size_t get_remaining_bits(UtDeflateDecoder *self, UtObject *data,
                                 size_t *offset) {
  return self->bit_count + (ut_list_get_length(data) - *offset) * 8;
}

static uint8_t read_bit(UtDeflateDecoder *self, UtObject *data,
                        size_t *offset) {
  if (self->bit_count == 0) {
    self->bit_buffer = ut_uint8_list_get_element(data, *offset);
    self->bit_count = 8;
    (*offset)++;
  }

  uint8_t value = self->bit_buffer & 0x01;
  self->bit_buffer >>= 1;
  self->bit_count--;

  return value;
}

static bool read_block_header(UtDeflateDecoder *self, UtObject *data,
                              size_t *offset) {
  size_t remaining = get_remaining_bits(self, data, offset);
  if (remaining < 3) {
    return false;
  }

  self->is_last_block = read_bit(self, data, offset) == 1;
  uint8_t block_type =
      read_bit(self, data, offset) << 1 | read_bit(self, data, offset);
  switch (block_type) {
  case 0:
    // Clear remaining unused bits
    self->bit_buffer = 0;
    self->bit_count = 0;
    self->state = DECODER_STATE_UNCOMPRESSED_LENGTH;
    return true;
  case 1:
    // FIXME: Dynamic Huffman codes
    assert(false);
  case 2:
    UtObjectRef distance_symbol_lengths = ut_uint8_list_new();
    for (size_t symbol = 0; symbol < 30; symbol++) {
      ut_uint8_list_append(distance_symbol_lengths, 5);
    }
    self->distance_decoder = ut_huffman_decoder_new(distance_symbol_lengths);

    UtObjectRef literal_or_length_symbol_lengths = ut_uint8_list_new();
    for (size_t symbol = 0; symbol <= 143; symbol++) {
      ut_uint8_list_append(literal_or_length_symbol_lengths, 8);
    }
    for (size_t symbol = 144; symbol <= 255; symbol++) {
      ut_uint8_list_append(literal_or_length_symbol_lengths, 9);
    }
    for (size_t symbol = 256; symbol <= 279; symbol++) {
      ut_uint8_list_append(literal_or_length_symbol_lengths, 7);
    }
    for (size_t symbol = 280; symbol <= 287; symbol++) {
      ut_uint8_list_append(literal_or_length_symbol_lengths, 8);
    }
    self->literal_or_length_decoder = ut_huffman_decoder_new(literal_or_length_symbol_lengths);

    self->state = DECODER_STATE_LITERAL_OR_LENGTH;
    return true;
  default:
    // Reserved type
    assert(false);
  }
}

static bool read_uncompressed_length(UtDeflateDecoder *self, UtObject *data,
                                     size_t *offset) {
  size_t remaining = ut_list_get_length(data) - *offset;
  if (remaining < 4) {
    return false;
  }

  self->length = ut_uint8_list_get_uint16_le(data, *offset);
  uint16_t nlength = ut_uint8_list_get_uint16_le(data, *offset + 2);
  assert((self->length ^ nlength) == 0xffff);

  self->state = DECODER_STATE_UNCOMPRESSED_DATA;
  *offset += 4;
  return true;
}

static bool read_uncompressed_data(UtDeflateDecoder *self, UtObject *data,
                                   size_t *offset) {
  size_t remaining = ut_list_get_length(data) - *offset;
  if (remaining < self->length) {
    return false;
  }

  for (size_t i = 0; i < self->length; i++) {
    ut_uint8_list_append(self->buffer,
                         ut_uint8_list_get_element(data, *offset + i));
  }

  *offset += self->length;
  self->state =
      self->is_last_block ? DECODER_STATE_DONE : DECODER_STATE_BLOCK_HEADER;
  return true;
}

static bool read_literal_or_length(UtDeflateDecoder *self, UtObject *data,
                                   size_t *offset) {
  size_t remaining = get_remaining_bits(self, data, offset);

  size_t min_code_width =
      ut_huffman_decoder_get_min_code_width(self->literal_or_length_decoder);
  if (remaining < min_code_width) {
    return false;
  }
  uint16_t code = 0;
  for (size_t i = 0; i < min_code_width; i++) {
    code = code << 1 | read_bit(self, data, offset);
  }
  size_t code_width = min_code_width;

  size_t max_code_width =
      ut_huffman_decoder_get_max_code_width(self->literal_or_length_decoder);
  uint16_t symbol;
  while (code_width <= max_code_width &&
         !ut_huffman_decoder_get_symbol(self->literal_or_length_decoder, code,
                                        code_width, &symbol)) {
    code = code << 1 | read_bit(self, data, offset);
    code_width++;
  }

  if (code_width > max_code_width) {
    // FIXME: Error
    return false;
  }

  if (symbol < 256) {
    ut_uint8_list_append(self->buffer, symbol);
    return true;
  } else if (symbol == 256) {
    self->state =
        self->is_last_block ? DECODER_STATE_DONE : DECODER_STATE_BLOCK_HEADER;
    return true;
  } else {
    self->state = DECODER_STATE_LENGTH;
    assert(symbol >= 257 && symbol <= 285);
    self->length = base_lengths[symbol - 257];
    self->extra_length_bits = extra_length_bits[symbol - 257];
    return true;
  }
}

static bool read_length(UtDeflateDecoder *self, UtObject *data,
                        size_t *offset) {
  uint16_t extra = 0;

  size_t remaining = get_remaining_bits(self, data, offset);
  if (remaining < self->extra_length_bits) {
    return false;
  }

  for (uint8_t i = 0; i < self->extra_length_bits; i++) {
    extra = extra << 1 | read_bit(self, data, offset);
  }
  self->length += extra;

  self->state = DECODER_STATE_DISTANCE;
  return true;
}

static bool read_distance(UtDeflateDecoder *self, UtObject *data,
                          size_t *offset) {
  size_t remaining = get_remaining_bits(self, data, offset);
  if (remaining < 5) {
    return false;
  }

  uint16_t c = 0;
  for (size_t i = 0; i < 5; i++) {
    c = c << 1 | read_bit(self, data, offset);
  }
  uint16_t code;
  assert(ut_huffman_decoder_get_symbol(self->distance_decoder, c, 5, &code));
  uint16_t extra = 0;
  uint8_t bit_count = distance_bits[code];
  for (uint8_t i = 0; i < bit_count; i++) {
    extra = extra << 1 | read_bit(self, data, offset);
  }
  uint16_t distance = base_distances[code] + extra;

  size_t buffer_length = ut_list_get_length(self->buffer);
  assert(distance <= buffer_length);
  size_t start = buffer_length - distance;
  for (size_t i = 0; i < self->length; i++) {
    ut_uint8_list_append(self->buffer,
                         ut_uint8_list_get_element(self->buffer, start + i));
  }

  self->state = DECODER_STATE_LITERAL_OR_LENGTH;

  return true;
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtDeflateDecoder *self = user_data;

  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->read_cancel);
    return 0;
  }

  size_t offset = 0;
  bool decoding = true;
  while (decoding) {
    switch (self->state) {
    case DECODER_STATE_BLOCK_HEADER:
      decoding = read_block_header(self, data, &offset);
      break;
    case DECODER_STATE_UNCOMPRESSED_LENGTH:
      decoding = read_uncompressed_length(self, data, &offset);
      break;
    case DECODER_STATE_UNCOMPRESSED_DATA:
      decoding = read_uncompressed_data(self, data, &offset);
      break;
    case DECODER_STATE_LITERAL_OR_LENGTH:
      decoding = read_literal_or_length(self, data, &offset);
      break;
    case DECODER_STATE_LENGTH:
      decoding = read_length(self, data, &offset);
      break;
    case DECODER_STATE_DISTANCE:
      decoding = read_distance(self, data, &offset);
      break;
    case DECODER_STATE_DONE:
      ut_cancel_activate(self->read_cancel);
      decoding = false;
      break;
    }
  }

  do {
    size_t n_used = self->callback(self->user_data, self->buffer,
                                   self->state == DECODER_STATE_DONE);
    ut_list_remove(self->buffer, 0, n_used);
  } while (ut_list_get_length(self->buffer) > 0 &&
           !ut_cancel_is_active(self->cancel));

  return offset;
}

static void ut_deflate_decoder_init(UtObject *object) {
  UtDeflateDecoder *self = (UtDeflateDecoder *)object;
  self->input_stream = NULL;
  self->read_cancel = ut_cancel_new();
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
  self->bit_buffer = 0x00;
  self->bit_count = 0;
  self->state = DECODER_STATE_BLOCK_HEADER;
  self->length = 0;
  self->extra_length_bits = 0;
  self->literal_or_length_decoder = NULL;
  self->distance_decoder = NULL;
  self->buffer = ut_uint8_array_new();
}

static void ut_deflate_decoder_cleanup(UtObject *object) {
  UtDeflateDecoder *self = (UtDeflateDecoder *)object;
  ut_cancel_activate(self->read_cancel);
  ut_object_unref(self->input_stream);
  ut_object_unref(self->read_cancel);
  ut_object_unref(self->cancel);
  ut_object_unref(self->buffer);
  ut_object_unref(self->literal_or_length_decoder);
  ut_object_unref(self->distance_decoder);
}

static void ut_deflate_decoder_read(UtObject *object,
                                    UtInputStreamCallback callback,
                                    void *user_data, UtObject *cancel) {
  UtDeflateDecoder *self = (UtDeflateDecoder *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);
  ut_input_stream_read(self->input_stream, read_cb, self, self->read_cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_deflate_decoder_read};

static UtObjectInterface object_interface = {
    .type_name = "UtDeflateDecoder",
    .init = ut_deflate_decoder_init,
    .cleanup = ut_deflate_decoder_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_deflate_decoder_new(UtObject *input_stream) {
  assert(input_stream != NULL);
  UtObject *object = ut_object_new(sizeof(UtDeflateDecoder), &object_interface);
  UtDeflateDecoder *self = (UtDeflateDecoder *)object;
  self->input_stream = ut_object_ref(input_stream);
  return object;
}

bool ut_object_is_deflate_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
