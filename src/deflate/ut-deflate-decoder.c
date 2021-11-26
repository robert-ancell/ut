#include <assert.h>

#include "ut-cancel.h"
#include "ut-constant-uint8-array.h"
#include "ut-deflate-decoder.h"
#include "ut-deflate-error.h"
#include "ut-huffman-decoder.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-uint16-list.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef enum {
  DECODER_STATE_BLOCK_HEADER,
  DECODER_STATE_UNCOMPRESSED_LENGTH,
  DECODER_STATE_UNCOMPRESSED_DATA,
  DECODER_STATE_HUFFMAN_LENGTHS,
  DECODER_STATE_HUFFMAN_WIDTHS,
  DECODER_STATE_HUFFMAN_LITERAL_OR_LENGTH_SYMBOL,
  DECODER_STATE_HUFFMAN_LITERAL_OR_LENGTH,
  DECODER_STATE_HUFFMAN_DISTANCE_SYMBOL,
  DECODER_STATE_HUFFMAN_DISTANCE,
  DECODER_STATE_LITERAL_OR_LENGTH,
  DECODER_STATE_LENGTH,
  DECODER_STATE_DISTANCE_SYMBOL,
  DECODER_STATE_DISTANCE,
  DECODER_STATE_DONE,
  DECODER_STATE_ERROR
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
  UtObject *error;
  bool is_last_block;
  uint16_t code;
  uint8_t code_width;
  uint16_t length;
  uint16_t length_symbol;
  uint16_t distance_symbol;

  // Huffman code used to encode the literal/length/distance codes.
  uint8_t n_huffman_width_symbols;
  UtObject *huffman_width_decoder;

  // Huffman code to encode literals and lengths.
  uint16_t n_literal_or_length_symbols;
  UtObject *literal_or_length_code_widths;

  // Huffman code to encode distances.
  uint8_t n_distance_symbols;
  UtObject *distance_code_widths;

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
    self->state = DECODER_STATE_HUFFMAN_LENGTHS;
    return true;
  case 2: {
    UtObjectRef literal_or_length_symbols = ut_uint16_list_new();
    UtObjectRef literal_or_length_code_widths = ut_uint8_list_new();
    for (size_t symbol = 0; symbol <= 287; symbol++) {
      ut_uint16_list_append(literal_or_length_symbols, symbol);
      uint8_t code_width;
      if (symbol <= 143) {
        code_width = 8;
      } else if (symbol <= 255) {
        code_width = 9;
      } else if (symbol <= 279) {
        code_width = 7;
      } else {
        code_width = 8;
      }
      ut_uint8_list_append(literal_or_length_code_widths, code_width);
    }
    ut_object_unref(self->literal_or_length_decoder);
    self->literal_or_length_decoder = ut_huffman_decoder_new(
        literal_or_length_symbols, literal_or_length_code_widths);

    UtObjectRef distance_symbols = ut_uint16_list_new();
    UtObjectRef distance_code_widths = ut_uint8_list_new();
    for (size_t symbol = 0; symbol < 30; symbol++) {
      ut_uint16_list_append(distance_symbols, symbol);
      ut_uint8_list_append(distance_code_widths, 5);
    }
    ut_object_unref(self->distance_decoder);
    self->distance_decoder =
        ut_huffman_decoder_new(distance_symbols, distance_code_widths);

    self->state = DECODER_STATE_LITERAL_OR_LENGTH;
    return true;
  }
  default:
    self->error = ut_deflate_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
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
  if ((self->length ^ nlength) != 0xffff) {
    self->error = ut_deflate_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
  }

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

static bool read_huffman_lengths(UtDeflateDecoder *self, UtObject *data,
                                 size_t *offset) {
  size_t remaining = get_remaining_bits(self, data, offset);
  if (remaining < 14) {
    return false;
  }

  self->n_literal_or_length_symbols =
      257 + (read_bit(self, data, offset) << 4 |
             read_bit(self, data, offset) << 3 |
             read_bit(self, data, offset) << 2 |
             read_bit(self, data, offset) << 1 | read_bit(self, data, offset));
  self->n_distance_symbols =
      1 + (read_bit(self, data, offset) << 4 |
           read_bit(self, data, offset) << 3 |
           read_bit(self, data, offset) << 2 |
           read_bit(self, data, offset) << 1 | read_bit(self, data, offset));
  self->n_huffman_width_symbols =
      4 + (read_bit(self, data, offset) << 3 |
           read_bit(self, data, offset) << 2 |
           read_bit(self, data, offset) << 1 | read_bit(self, data, offset));

  ut_object_unref(self->literal_or_length_code_widths);
  self->literal_or_length_code_widths = ut_uint8_list_new();
  ut_object_unref(self->distance_code_widths);
  self->distance_code_widths = ut_uint8_list_new();

  self->state = DECODER_STATE_HUFFMAN_WIDTHS;
  return true;
}

static bool read_huffman_widths(UtDeflateDecoder *self, UtObject *data,
                                size_t *offset) {
  size_t remaining = get_remaining_bits(self, data, offset);
  if (remaining < self->n_huffman_width_symbols * 3) {
    return false;
  }

  uint8_t code_widths[19] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (size_t i = 0; i < self->n_huffman_width_symbols; i++) {
    uint8_t symbols[19] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                           11, 4,  12, 3, 13, 2, 14, 1, 15};
    code_widths[symbols[i]] = read_bit(self, data, offset) << 2 |
                              read_bit(self, data, offset) << 1 |
                              read_bit(self, data, offset);
  }
  UtObjectRef code_widths_array = ut_constant_uint8_array_new(code_widths, 19);
  self->huffman_width_decoder = ut_huffman_decoder_new(code_widths_array);

  self->state = DECODER_STATE_HUFFMAN_LITERAL_OR_LENGTH;
  return true;
}

static bool read_huffman_symbol(UtDeflateDecoder *self, UtObject *data,
                                size_t *offset, UtObject *decoder,
                                uint16_t *symbol) {
  size_t remaining = get_remaining_bits(self, data, offset);

  if (self->code_width == 0) {
    size_t min_code_width = ut_huffman_decoder_get_min_code_width(decoder);
    if (remaining < min_code_width) {
      return false;
    }
    self->code = 0;
    for (size_t i = 0; i < min_code_width; i++) {
      self->code = self->code << 1 | read_bit(self, data, offset);
    }
    self->code_width = min_code_width;
  }

  size_t max_code_width = ut_huffman_decoder_get_max_code_width(decoder);
  while (self->code_width < remaining && self->code_width <= max_code_width &&
         !ut_huffman_decoder_get_symbol(decoder, self->code, self->code_width,
                                        symbol)) {
    self->code = self->code << 1 | read_bit(self, data, offset);
    self->code_width++;
  }

  if (self->code_width >= remaining) {
    return false;
  }

  if (self->code_width > max_code_width) {
    self->error = ut_deflate_error_new();
    self->state = DECODER_STATE_ERROR;
    return false;
  }

  self->code = 0;
  self->code_width = 0;

  return true;
}

static bool read_huffman_literal_or_length_symbol(UtDeflateDecoder *self,
                                                  UtObject *data,
                                                  size_t *offset) {

  if (!read_huffman_symbol(self, data, offset, self->huffman_width_decoder,
                           &self->length_symbol)) {
    return self->error != NULL;
  }

  self->state = DECODER_STATE_HUFFMAN_LITERAL_OR_LENGTH;

  return true;
}

static bool read_widths(UtDeflateDecoder *self, UtObject *data, size_t *offset,
                        uint16_t symbol, UtObject *code_widths) {
  if (symbol <= 15) {
    ut_uint8_list_append(code_widths, symbol);
  } else {
    size_t bit_count;
    size_t base_count;
    if (symbol == 16) {
      base_count = 3;
      bit_count = 2;
    } else if (symbol == 17) {
      base_count = 3;
      bit_count = 3;
    } else if (symbol == 18) {
      base_count = 11;
      bit_count = 7;
    } else {
      assert(false);
    }

    size_t remaining = get_remaining_bits(self, data, offset);
    if (remaining < bit_count) {
      return false;
    }

    uint16_t extra = 0;
    for (uint8_t i = 0; i < bit_count; i++) {
      extra = extra << 1 | read_bit(self, data, offset);
    }

    size_t code_widths_length = ut_list_get_length(code_widths);
    if (code_widths_length == 0) {
      self->error = ut_deflate_error_new();
      self->state = DECODER_STATE_ERROR;
      return true;
    }
    uint8_t code_width =
        ut_uint8_list_get_element(code_widths, code_widths_length - 1);
    size_t count = base_count + extra;
    for (size_t i = 0; i < count; i++) {
      ut_uint8_list_append(code_widths, code_width);
    }
  }

  return true;
}

static bool read_huffman_literal_or_length(UtDeflateDecoder *self,
                                           UtObject *data, size_t *offset) {
  if (!read_widths(self, data, offset, self->length_symbol,
                   self->literal_or_length_code_widths)) {
    return self->error != NULL;
  }

  size_t code_widths_length =
      ut_list_get_length(self->literal_or_length_code_widths);
  if (code_widths_length < self->n_literal_or_length_symbols) {
    self->state = DECODER_STATE_HUFFMAN_LITERAL_OR_LENGTH_SYMBOL;
    return true;
  } else if (code_widths_length == self->n_literal_or_length_symbols) {
    ut_object_unref(self->literal_or_length_decoder);
    self->literal_or_length_decoder =
        ut_huffman_decoder_new(self->literal_or_length_code_widths);
    self->state = DECODER_STATE_HUFFMAN_DISTANCE_SYMBOL;
    return true;
  } else {
    self->error = ut_deflate_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
  }
}

static bool read_huffman_distance_symbol(UtDeflateDecoder *self, UtObject *data,
                                         size_t *offset) {

  if (!read_huffman_symbol(self, data, offset, self->huffman_width_decoder,
                           &self->distance_symbol)) {
    return self->error != NULL;
  }

  self->state = DECODER_STATE_HUFFMAN_DISTANCE;

  return true;
}

static bool read_huffman_distance(UtDeflateDecoder *self, UtObject *data,
                                  size_t *offset) {
  if (!read_widths(self, data, offset, self->distance_symbol,
                   self->distance_code_widths)) {
    return self->error != NULL;
  }

  size_t code_widths_length = ut_list_get_length(self->distance_code_widths);
  if (code_widths_length < self->n_distance_symbols) {
    self->state = DECODER_STATE_HUFFMAN_DISTANCE_SYMBOL;
    return true;
  } else if (code_widths_length == self->n_distance_symbols) {
    ut_object_unref(self->distance_decoder);
    self->distance_decoder = ut_huffman_decoder_new(self->distance_code_widths);
    self->state = DECODER_STATE_LITERAL_OR_LENGTH;
    return true;
  } else {
    self->error = ut_deflate_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
  }
}

static bool read_literal_or_length(UtDeflateDecoder *self, UtObject *data,
                                   size_t *offset) {
  uint16_t symbol;
  if (!read_huffman_symbol(self, data, offset, self->literal_or_length_decoder,
                           &symbol)) {
    return self->error != NULL;
  }

  if (symbol < 256) {
    ut_uint8_list_append(self->buffer, symbol);
    return true;
  } else if (symbol == 256) {
    self->state =
        self->is_last_block ? DECODER_STATE_DONE : DECODER_STATE_BLOCK_HEADER;
    return true;
  } else if (symbol <= 285) {
    self->state = DECODER_STATE_LENGTH;
    self->length_symbol = symbol;
    return true;
  } else {
    self->error = ut_deflate_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
  }
}

static bool read_length(UtDeflateDecoder *self, UtObject *data,
                        size_t *offset) {
  size_t remaining = get_remaining_bits(self, data, offset);

  uint8_t bit_count = extra_length_bits[self->length_symbol - 257];
  if (remaining < bit_count) {
    return false;
  }

  uint16_t extra = 0;
  for (uint8_t i = 0; i < bit_count; i++) {
    extra = extra << 1 | read_bit(self, data, offset);
  }
  self->length = base_lengths[self->length_symbol - 257] + extra;

  self->state = DECODER_STATE_DISTANCE_SYMBOL;
  return true;
}

static bool read_distance_symbol(UtDeflateDecoder *self, UtObject *data,
                                 size_t *offset) {
  if (!read_huffman_symbol(self, data, offset, self->distance_decoder,
                           &self->distance_symbol)) {
    return self->error != NULL;
  }

  self->state = DECODER_STATE_DISTANCE;

  return true;
}

static bool read_distance(UtDeflateDecoder *self, UtObject *data,
                          size_t *offset) {
  size_t remaining = get_remaining_bits(self, data, offset);

  uint8_t bit_count = distance_bits[self->distance_symbol];
  if (remaining < bit_count) {
    return false;
  }

  uint16_t extra = 0;
  for (uint8_t i = 0; i < bit_count; i++) {
    extra = extra << 1 | read_bit(self, data, offset);
  }
  uint16_t distance = base_distances[self->distance_symbol] + extra;

  size_t buffer_length = ut_list_get_length(self->buffer);
  if (distance > buffer_length) {
    self->error = ut_deflate_error_new();
    self->state = DECODER_STATE_ERROR;
    return true;
  }
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
    case DECODER_STATE_HUFFMAN_LENGTHS:
      decoding = read_huffman_lengths(self, data, &offset);
      break;
    case DECODER_STATE_HUFFMAN_WIDTHS:
      decoding = read_huffman_widths(self, data, &offset);
      break;
    case DECODER_STATE_HUFFMAN_LITERAL_OR_LENGTH_SYMBOL:
      decoding = read_huffman_literal_or_length_symbol(self, data, &offset);
      break;
    case DECODER_STATE_HUFFMAN_LITERAL_OR_LENGTH:
      decoding = read_huffman_literal_or_length(self, data, &offset);
      break;
    case DECODER_STATE_HUFFMAN_DISTANCE_SYMBOL:
      decoding = read_huffman_distance_symbol(self, data, &offset);
      break;
    case DECODER_STATE_HUFFMAN_DISTANCE:
      decoding = read_huffman_distance(self, data, &offset);
      break;
    case DECODER_STATE_LITERAL_OR_LENGTH:
      decoding = read_literal_or_length(self, data, &offset);
      break;
    case DECODER_STATE_LENGTH:
      decoding = read_length(self, data, &offset);
      break;
    case DECODER_STATE_DISTANCE_SYMBOL:
      decoding = read_distance_symbol(self, data, &offset);
      break;
    case DECODER_STATE_DISTANCE:
      decoding = read_distance(self, data, &offset);
      break;
    case DECODER_STATE_DONE:
      ut_cancel_activate(self->read_cancel);
      decoding = false;
      break;
    case DECODER_STATE_ERROR:
      ut_cancel_activate(self->read_cancel);
      self->callback(self->user_data, self->error, true);
      return offset;
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
  self->error = NULL;
  self->is_last_block = false;
  self->code = 0;
  self->code_width = 0;
  self->length = 0;
  self->length_symbol = 0;
  self->distance_symbol = 0;
  self->n_huffman_width_symbols = 0;
  self->huffman_width_decoder = NULL;
  self->n_literal_or_length_symbols = 0;
  self->literal_or_length_code_widths = NULL;
  self->n_distance_symbols = 0;
  self->distance_code_widths = NULL;
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
  ut_object_unref(self->error);
  ut_object_unref(self->huffman_width_decoder);
  ut_object_unref(self->literal_or_length_decoder);
  ut_object_unref(self->literal_or_length_code_widths);
  ut_object_unref(self->distance_code_widths);
  ut_object_unref(self->distance_decoder);
  ut_object_unref(self->buffer);
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
