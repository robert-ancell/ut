#include <assert.h>
#include <stdint.h>

#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-general-error.h"
#include "ut-huffman-decoder.h"
#include "ut-input-stream.h"
#include "ut-jpeg-decoder.h"
#include "ut-jpeg-error.h"
#include "ut-jpeg-image.h"
#include "ut-list-input-stream.h"
#include "ut-list.h"
#include "ut-uint16-list.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

// https://www.w3.org/Graphics/JPEG/itu-t81.pdf
// https://www.w3.org/Graphics/JPEG/jfif3.pdf

// FIXME
#include <stdio.h>

typedef enum {
  DECODER_STATE_MARKER,
  DECODER_STATE_START_OF_IMAGE,
  DECODER_STATE_END_OF_IMAGE,
  DECODER_STATE_QUANTIZATION_TABLE,
  DECODER_STATE_BASELINE_DCT,
  DECODER_STATE_HUFFMAN_TABLE,
  DECODER_STATE_START_OF_SCAN,
  DECODER_STATE_APP0,
  DECODER_STATE_COMMENT,
  DECODER_STATE_JPEG_DATA,
  DECODER_STATE_DONE,
  DECODER_STATE_ERROR
} DecoderState;

typedef struct {
  UtObject object;
  UtObject *input_stream;
  UtObject *read_cancel;

  UtJpegDecodeCallback callback;
  void *user_data;
  UtObject *cancel;

  uint8_t bit_buffer;
  uint8_t bit_count;

  DecoderState state;
  UtObject *error;
  UtObject *ac_decoders[16];
  UtObject *dc_decoders[16];

  uint16_t code;
  uint8_t code_width;

  uint8_t coefficients[64];
  uint8_t coefficient_index;

  uint16_t thumbnail_width;
  uint16_t thumbnail_height;
  UtObject *thumbnail_data;
  uint16_t width;
  uint16_t height;
  char *comment;
  UtObject *data;
} UtJpegDecoder;

static size_t decode_start_of_image(UtJpegDecoder *self, UtObject *data) {
  printf("soi\n");
  self->state = DECODER_STATE_MARKER;
  return 0;
}

static size_t decode_end_of_image(UtJpegDecoder *self, UtObject *data) {
  printf("eoi\n");
  self->state = DECODER_STATE_DONE;
  return 0;
}

static size_t decode_app0(UtJpegDecoder *self, UtObject *data) {
  size_t data_length = ut_list_get_length(data);

  if (data_length < 2) {
    return 0;
  }
  uint16_t length = ut_uint8_list_get_uint16_be(data, 0);
  if (data_length < length) {
    return 0;
  }

  if (length < 16) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  // FIXME: Also support 'JFXX'
  if (ut_uint8_list_get_element(data, 2) != 'J' ||
      ut_uint8_list_get_element(data, 3) != 'F' ||
      ut_uint8_list_get_element(data, 4) != 'I' ||
      ut_uint8_list_get_element(data, 5) != 'F' ||
      ut_uint8_list_get_element(data, 6) != '\0') {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  uint8_t jpeg_version_major = ut_uint8_list_get_element(data, 7);
  /*uint8_t jpeg_version_minor =*/ut_uint8_list_get_element(data, 8);
  if (jpeg_version_major != 1) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  /*uint8_t density_units =*/ut_uint8_list_get_element(data, 9);
  /*uint16_t x_density=*/ut_uint8_list_get_uint16_be(data, 10);
  /*uint16_t x_density=*/ut_uint8_list_get_uint16_be(data, 12);
  self->thumbnail_width = ut_uint8_list_get_element(data, 14);
  self->thumbnail_height = ut_uint8_list_get_element(data, 15);
  size_t thumbnail_size = self->thumbnail_width * self->thumbnail_height * 3;
  if (length < 16 + thumbnail_size) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }
  ut_object_unref(self->thumbnail_data);
  self->thumbnail_data = thumbnail_size > 0 ? ut_uint8_array_new() : NULL;
  for (size_t i = 0; i < thumbnail_size; i++) {
    ut_uint8_list_append(self->thumbnail_data,
                         ut_uint8_list_get_element(data, 16 + i));
  }

  printf("app0 thumbnail=%dx%d\n", self->thumbnail_width,
         self->thumbnail_height);

  self->state = DECODER_STATE_MARKER;

  return length;
}

static size_t decode_quantization_table(UtJpegDecoder *self, UtObject *data) {
  size_t data_length = ut_list_get_length(data);

  if (data_length < 2) {
    return 0;
  }
  uint16_t length = ut_uint8_list_get_uint16_be(data, 0);
  if (data_length < length) {
    return 0;
  }

  if (length != 67) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  uint8_t precision_and_number = ut_uint8_list_get_element(data, 2);
  uint8_t precision = precision_and_number >> 4;
  uint8_t number = precision_and_number & 0xf;
  printf("qt precision=%d number=%d\n", precision, number);

  self->state = DECODER_STATE_MARKER;

  return length;
}

static size_t decode_baseline_dct(UtJpegDecoder *self, UtObject *data) {
  size_t data_length = ut_list_get_length(data);

  if (data_length < 2) {
    return 0;
  }
  uint16_t length = ut_uint8_list_get_uint16_be(data, 0);
  if (data_length < length) {
    return 0;
  }

  if (length < 6) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  self->height = ut_uint8_list_get_uint16_be(data, 3);
  self->width = ut_uint8_list_get_uint16_be(data, 5);

  printf("sof %dx%d\n", self->width, self->height);

  self->state = DECODER_STATE_MARKER;

  return length;
}

static size_t decode_huffman_table(UtJpegDecoder *self, UtObject *data) {
  size_t data_length = ut_list_get_length(data);

  if (data_length < 2) {
    return 0;
  }
  uint16_t length = ut_uint8_list_get_uint16_be(data, 0);
  if (data_length < length) {
    return 0;
  }

  if (length < 19) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  uint8_t type_and_number = ut_uint8_list_get_element(data, 2);
  uint8_t type = type_and_number >> 4;
  uint8_t number = type_and_number & 0xf;
  printf("huff type=%d number=%d\n", type, number);

  if (type > 1) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  size_t symbol_data_length = 0;
  for (size_t i = 0; i < 16; i++) {
    symbol_data_length += ut_uint8_list_get_element(data, 3 + i);
  }
  if (length != 19 + symbol_data_length) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  UtObjectRef symbols = ut_uint16_list_new();
  UtObjectRef code_widths = ut_uint8_list_new();
  size_t s = 0;
  for (size_t code_width = 1; code_width <= 16; code_width++) {
    uint8_t symbol_count = ut_uint8_list_get_element(data, 3 + code_width - 1);
    for (size_t i = 0; i < symbol_count; i++) {
      uint8_t symbol = ut_uint8_list_get_element(data, 19 + s);
      ut_uint16_list_append(symbols, symbol);
      ut_uint8_list_append(code_widths, code_width);
      s++;
    }
  }

  UtObject *decoder = ut_huffman_decoder_new(symbols, code_widths);
  if (type == 0) {
    ut_object_unref(self->dc_decoders[number]);
    self->dc_decoders[number] = decoder;
  } else {
    ut_object_unref(self->ac_decoders[number]);
    self->ac_decoders[number] = decoder;
  }

  self->state = DECODER_STATE_MARKER;

  return length;
}

static size_t decode_start_of_scan(UtJpegDecoder *self, UtObject *data) {
  size_t data_length = ut_list_get_length(data);

  if (data_length < 2) {
    return 0;
  }
  uint16_t length = ut_uint8_list_get_uint16_be(data, 0);
  if (data_length < length) {
    return 0;
  }

  uint8_t n_components = ut_uint8_list_get_element(data, 2);
  if (n_components < 1 || n_components > 4) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }
  if (length != 6 + n_components * 2) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return length;
  }

  printf("sos\n");
  size_t offset = 3;
  for (size_t i = 0; i < n_components; i++) {
    uint8_t cs = ut_uint8_list_get_element(data, offset);
    uint8_t tables = ut_uint8_list_get_element(data, offset + 1);
    uint8_t dc_table = tables >> 4;
    uint8_t ac_table = tables & 0xf;
    offset += 2;

    printf("  cs=%d dc=%d ac=%d\n", cs, dc_table, ac_table);
  }

  self->coefficient_index = 0;
  self->state = DECODER_STATE_JPEG_DATA;

  return length;
}

static size_t decode_comment(UtJpegDecoder *self, UtObject *data) {
  size_t data_length = ut_list_get_length(data);

  if (data_length < 2) {
    return 0;
  }
  uint16_t length = ut_uint8_list_get_uint16_be(data, 0);
  if (data_length < length) {
    return 0;
  }

  UtObjectRef comment_data = ut_uint8_array_new();
  for (size_t i = 2; i < length; i++) {
    ut_uint8_list_append(comment_data, ut_uint8_list_get_element(data, i));
  }
  ut_uint8_list_append(comment_data, '\0');
  free(self->comment);
  self->comment = (char *)ut_uint8_list_take_data(comment_data);
  printf("com \"%s\"\n", self->comment);

  self->state = DECODER_STATE_MARKER;

  return length;
}

static size_t get_remaining_bits(UtJpegDecoder *self, UtObject *data,
                                 size_t *offset) {
  return self->bit_count + (ut_list_get_length(data) - *offset) * 8;
}

static uint8_t read_bit(UtJpegDecoder *self, UtObject *data, size_t *offset) {
  if (self->bit_count == 0) {
    self->bit_buffer = ut_uint8_list_get_element(data, *offset);
    self->bit_count = 8;
    (*offset)++;
  }

  uint8_t value = self->bit_buffer >> 7;
  self->bit_buffer <<= 1;
  self->bit_count--;

  return value;
}

static bool read_huffman_symbol(UtJpegDecoder *self, UtObject *data,
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
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return false;
  }

  self->code = 0;
  self->code_width = 0;

  return true;
}

static size_t decode_jpeg_data(UtJpegDecoder *self, UtObject *data,
                               bool complete) {
  printf("data %zi %d\n", ut_list_get_length(data), complete);

  size_t offset = 0;
  UtObject *decoder = self->dc_decoders[0];
  while (true) {
    uint16_t symbol;
    if (!read_huffman_symbol(self, data, &offset, decoder, &symbol)) {
      printf("!\n");
      return offset;
    }
    if (decoder == self->dc_decoders[0]) {
      for (size_t i = 0; i < symbol; i++) {
        read_bit(self, data, &offset);
      }
    }
    decoder = self->ac_decoders[0];
    printf("  %d\n", symbol);
  }

  if (complete) {
    self->state = DECODER_STATE_DONE;
  }

  return ut_list_get_length(data);
}

static size_t decode_marker(UtJpegDecoder *self, UtObject *data) {
  size_t data_length = ut_list_get_length(data);

  if (data_length < 2) {
    return 0;
  }
  uint8_t marker_signature = ut_uint8_list_get_element(data, 0);
  uint8_t marker_id = ut_uint8_list_get_element(data, 1);
  if (marker_signature != 0xff) {
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    return 0;
  }

  switch (marker_id) {
  case 0xd8:
    self->state = DECODER_STATE_START_OF_IMAGE;
    break;
  case 0xd9:
    self->state = DECODER_STATE_END_OF_IMAGE;
    break;
  case 0xdb:
    self->state = DECODER_STATE_QUANTIZATION_TABLE;
    break;
  case 0xc0:
    self->state = DECODER_STATE_BASELINE_DCT;
    break;
  case 0xc4:
    self->state = DECODER_STATE_HUFFMAN_TABLE;
    break;
  case 0xda:
    self->state = DECODER_STATE_START_OF_SCAN;
    break;
  case 0xe0:
    self->state = DECODER_STATE_APP0;
    break;
  case 0xfe:
    self->state = DECODER_STATE_COMMENT;
    break;
  default:
    printf("unknown marker %02x\n", marker_id);
    self->error = ut_jpeg_error_new();
    self->state = DECODER_STATE_ERROR;
    break;
  }

  return 2;
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtJpegDecoder *self = user_data;

  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->read_cancel);
    return 0;
  }

  size_t data_length = ut_list_get_length(data);
  size_t offset = 0;
  while (true) {
    size_t n_used;
    UtObjectRef d = ut_list_get_sublist(data, offset, data_length - offset);
    DecoderState old_state = self->state;
    switch (self->state) {
    case DECODER_STATE_MARKER:
      n_used = decode_marker(self, d);
      break;
    case DECODER_STATE_START_OF_IMAGE:
      n_used = decode_start_of_image(self, d);
      break;
    case DECODER_STATE_END_OF_IMAGE:
      n_used = decode_end_of_image(self, d);
      break;
    case DECODER_STATE_QUANTIZATION_TABLE:
      n_used = decode_quantization_table(self, d);
      break;
    case DECODER_STATE_BASELINE_DCT:
      n_used = decode_baseline_dct(self, d);
      break;
    case DECODER_STATE_HUFFMAN_TABLE:
      n_used = decode_huffman_table(self, d);
      break;
    case DECODER_STATE_START_OF_SCAN:
      n_used = decode_start_of_scan(self, d);
      break;
    case DECODER_STATE_APP0:
      n_used = decode_app0(self, d);
      break;
    case DECODER_STATE_COMMENT:
      n_used = decode_comment(self, d);
      break;
    case DECODER_STATE_JPEG_DATA:
      n_used = decode_jpeg_data(self, d, complete);
      break;
    case DECODER_STATE_ERROR:
      ut_cancel_activate(self->read_cancel);
      self->callback(self->user_data, self->error);
      return offset;
    case DECODER_STATE_DONE:
      ut_cancel_activate(self->read_cancel);
      UtObjectRef image =
          ut_jpeg_image_new(self->width, self->height, self->data);
      ut_jpeg_image_set_thumbnail(image, self->thumbnail_width,
                                  self->thumbnail_height, self->thumbnail_data);
      self->callback(self->user_data, image);
      return offset;
    default:
      assert(false);
    }

    offset += n_used;
    if (self->state == old_state && (n_used == 0 || offset == data_length)) {
      return offset;
    }
  }
}

static void sync_cb(void *user_data, UtObject *image) {
  UtObject **result = user_data;
  assert(*result == NULL);
  *result = ut_object_ref(image);
}

static void ut_jpeg_decoder_init(UtObject *object) {
  UtJpegDecoder *self = (UtJpegDecoder *)object;
  self->input_stream = NULL;
  self->read_cancel = ut_cancel_new();
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
  self->bit_buffer = 0x00;
  self->bit_count = 0;
  self->state = DECODER_STATE_MARKER;
  self->error = NULL;
  for (size_t i = 0; i < 16; i++) {
    self->ac_decoders[i] = NULL;
    self->dc_decoders[i] = NULL;
  }
  self->code = 0;
  self->code_width = 0;
  for (size_t i = 0; i < 64; i++) {
    self->coefficients[i] = 0;
  }
  self->coefficient_index = 0;
  self->thumbnail_width = 0;
  self->thumbnail_height = 0;
  self->thumbnail_data = NULL;
  self->width = 0;
  self->height = 0;
  self->comment = NULL;
  self->data = ut_uint8_array_new();
}

static void ut_jpeg_decoder_cleanup(UtObject *object) {
  UtJpegDecoder *self = (UtJpegDecoder *)object;
  ut_cancel_activate(self->read_cancel);
  ut_object_unref(self->input_stream);
  ut_object_unref(self->read_cancel);
  ut_object_unref(self->cancel);
  ut_object_unref(self->error);
  for (size_t i = 0; i < 16; i++) {
    ut_object_unref(self->ac_decoders[i]);
    ut_object_unref(self->dc_decoders[i]);
  }
  free(self->comment);
  ut_object_unref(self->data);
}

static UtObjectInterface object_interface = {.type_name = "UtJpegDecoder",
                                             .init = ut_jpeg_decoder_init,
                                             .cleanup = ut_jpeg_decoder_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_jpeg_decoder_new(UtObject *input_stream) {
  UtObject *object = ut_object_new(sizeof(UtJpegDecoder), &object_interface);
  UtJpegDecoder *self = (UtJpegDecoder *)object;
  self->input_stream = ut_object_ref(input_stream);
  return object;
}

void ut_jpeg_decoder_decode(UtObject *object, UtJpegDecodeCallback callback,
                            void *user_data, UtObject *cancel) {
  assert(ut_object_is_jpeg_decoder(object));
  UtJpegDecoder *self = (UtJpegDecoder *)object;

  assert(self->callback == NULL);
  assert(callback != NULL);

  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);

  ut_input_stream_read(self->input_stream, read_cb, self, self->read_cancel);
}

UtObject *ut_jpeg_decoder_decode_sync(UtObject *object) {
  UtObject *result = NULL;
  UtObjectRef cancel = ut_cancel_new();
  ut_jpeg_decoder_decode(object, sync_cb, &result, cancel);
  ut_cancel_activate(cancel);
  if (result == NULL) {
    result = ut_general_error_new("Sync call did not complete");
  }
  return result;
}

bool ut_object_is_jpeg_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
