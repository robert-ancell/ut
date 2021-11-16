#include <assert.h>

#include "ut-cancel.h"
#include "ut-general-error.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-png-decoder.h"
#include "ut-png-error.h"
#include "ut-png-image.h"
#include "ut-uint8-list.h"

typedef enum {
  IMAGE_HEADER = 0x49484452,
  PALETTE = 0x504c5445,
  IMAGE_DATA = 0x49444154,
  IMAGE_END = 0x49454e44,
  CHROMATICITIES = 0x6348524d,
  GAMMA = 0x67414d41,
  ICC_PROFILE = 0x69434350,
  SIGNIFICANT_BITS = 0x73424954,
  STANDARD_RGB = 0x73524742,
  BACKGROUND = 0x624b4744,
  HISTOGRAM = 0x68495354,
  PHYSICAL_DIMENSIONS = 0x70485973,
  SUGGESTED_PALETTE = 0x73504c54,
  MODIFICATION_TIME = 0x74494d45,
  TEXT = 0x74455874,
  COMPRESSED_TEXT = 0x7a545874,
  INTERNATIONAL_TEXT = 0x69545874
} ChunkType;

typedef enum {
  DECODER_STATE_SIGNATURE,
  DECODER_STATE_ERROR,
  DECODER_STATE_CHUNK,
  DECODER_STATE_END
} DecoderState;

typedef struct {
  UtObject object;
  UtObject *input_stream;
  UtObject *read_cancel;
  UtPngDecodeCallback callback;
  void *user_data;
  UtObject *cancel;
  DecoderState state;
  UtObject *error;
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  UtPngColourType colour_type;
  uint8_t compression_method;
  uint8_t filter_method;
  uint8_t interlace_method;
} UtPngDecoder;

static size_t decode_signature(UtPngDecoder *self, UtObject *data,
                               size_t offset) {
  if (ut_list_get_length(data) - offset < 8) {
    return 0;
  }

  if (ut_uint8_list_get_element(data, offset + 0) != 137 ||
      ut_uint8_list_get_element(data, offset + 1) != 80 ||
      ut_uint8_list_get_element(data, offset + 2) != 78 ||
      ut_uint8_list_get_element(data, offset + 3) != 71 ||
      ut_uint8_list_get_element(data, offset + 4) != 13 ||
      ut_uint8_list_get_element(data, offset + 5) != 10 ||
      ut_uint8_list_get_element(data, offset + 6) != 26 ||
      ut_uint8_list_get_element(data, offset + 7) != 10) {
    self->error = ut_png_error_new();
    self->state = DECODER_STATE_ERROR;
    return 0;
  }

  self->state = DECODER_STATE_CHUNK;
  return 8;
}

static bool decode_colour_type(uint8_t value, UtPngColourType *type) {
  switch (value) {
  case 0:
    *type = UT_PNG_COLOUR_TYPE_GREYSCALE;
    return true;
  case 2:
    *type = UT_PNG_COLOUR_TYPE_TRUECOLOUR;
    return true;
  case 3:
    *type = UT_PNG_COLOUR_TYPE_INDEXED_COLOUR;
    return true;
  case 4:
    *type = UT_PNG_COLOUR_TYPE_GREYSCALE_WITH_ALPHA;
    return true;
  case 6:
    *type = UT_PNG_COLOUR_TYPE_TRUECOLOUR_WITH_ALPHA;
    return true;
  default:
    return false;
  }
}

static bool is_valid_bit_depth(uint8_t type, uint8_t depth) {
  switch (type) {
  case UT_PNG_COLOUR_TYPE_GREYSCALE:
    return depth == 1 || depth == 2 || depth == 4 || depth == 8 || depth == 16;
  case UT_PNG_COLOUR_TYPE_TRUECOLOUR:
    return depth == 8 || depth == 16;
  case UT_PNG_COLOUR_TYPE_INDEXED_COLOUR:
    return depth == 1 || depth == 2 || depth == 4 || depth == 8;
  case UT_PNG_COLOUR_TYPE_GREYSCALE_WITH_ALPHA:
    return depth == 8 || depth == 16;
  case UT_PNG_COLOUR_TYPE_TRUECOLOUR_WITH_ALPHA:
    return depth == 8 || depth == 16;
  default:
    return false;
  }
}

static bool is_valid_compression_method(uint8_t method) { return method == 0; }

static bool is_valid_filter_method(uint8_t method) { return method == 0; }

static bool is_valid_interlace_method(uint8_t method) {
  return method == 0 || method == 1;
}

static void decode_image_header(UtPngDecoder *self, UtObject *data,
                                size_t offset, size_t length) {
  if (length != 13) {
    self->error = ut_png_error_new();
    self->state = DECODER_STATE_ERROR;
    return;
  }

  self->width = ut_uint8_list_get_uint32_be(data, offset);
  self->height = ut_uint8_list_get_uint32_be(data, offset + 4);
  self->bit_depth = ut_uint8_list_get_element(data, offset + 8);
  bool valid_colour_type = decode_colour_type(
      ut_uint8_list_get_element(data, offset + 9), &self->colour_type);
  self->compression_method = ut_uint8_list_get_element(data, offset + 10);
  self->filter_method = ut_uint8_list_get_element(data, offset + 11);
  self->interlace_method = ut_uint8_list_get_element(data, offset + 12);

  if (self->width == 0 || self->height == 0 || !valid_colour_type ||
      !is_valid_bit_depth(self->colour_type, self->bit_depth) ||
      !is_valid_compression_method(self->compression_method) ||
      !is_valid_filter_method(self->filter_method) ||
      !is_valid_interlace_method(self->interlace_method)) {
    self->error = ut_png_error_new();
    self->state = DECODER_STATE_ERROR;
    return;
  }
}

static void decode_palette(UtPngDecoder *self, UtObject *data, size_t offset,
                           size_t length) {}

static void decode_image_data(UtPngDecoder *self, UtObject *data, size_t offset,
                              size_t length) {}

static void decode_image_end(UtPngDecoder *self, UtObject *data, size_t offset,
                             size_t length) {
  if (length != 0) {
    self->error = ut_png_error_new();
    self->state = DECODER_STATE_ERROR;
    return;
  }

  self->state = DECODER_STATE_END;
}

static void decode_background(UtPngDecoder *self, UtObject *data, size_t offset,
                              size_t length) {
  switch (self->colour_type) {
  case UT_PNG_COLOUR_TYPE_GREYSCALE:
  case UT_PNG_COLOUR_TYPE_GREYSCALE_WITH_ALPHA:
    if (length != 2) {
      self->error = ut_png_error_new();
      self->state = DECODER_STATE_ERROR;
      return;
    }
    /*self->background_colour = */ ut_uint8_list_get_uint16_be(data, offset);
    break;
  case UT_PNG_COLOUR_TYPE_TRUECOLOUR:
  case UT_PNG_COLOUR_TYPE_TRUECOLOUR_WITH_ALPHA:
    if (length != 6) {
      self->error = ut_png_error_new();
      self->state = DECODER_STATE_ERROR;
      /*self->background_colour_r = */ ut_uint8_list_get_uint16_be(data,
                                                                   offset);
      /*self->background_colour_g = */ ut_uint8_list_get_uint16_be(data,
                                                                   offset + 2);
      /*self->background_colour_b = */ ut_uint8_list_get_uint16_be(data,
                                                                   offset + 4);
      return;
    }
    break;
  case UT_PNG_COLOUR_TYPE_INDEXED_COLOUR:
    if (length != 1) {
      self->error = ut_png_error_new();
      self->state = DECODER_STATE_ERROR;
      return;
    }
    /*self->background_colour_b = */ ut_uint8_list_get_element(data, offset);
    break;
  default:
    assert(false);
  }
}

static void decode_physical_dimensions(UtPngDecoder *self, UtObject *data,
                                       size_t offset, size_t length) {
  if (length != 9) {
    self->error = ut_png_error_new();
    self->state = DECODER_STATE_ERROR;
    return;
  }

  /*self->pixels_per_unit_x =*/ut_uint8_list_get_uint32_be(data, offset + 0);
  /*self->pixels_per_unix_y =*/ut_uint8_list_get_uint32_be(data, offset + 4);
  /*self->unit_specifier =*/ut_uint8_list_get_element(data, offset + 8);
}

static void decode_modification_time(UtPngDecoder *self, UtObject *data,
                                     size_t offset, size_t length) {
  if (length != 7) {
    self->error = ut_png_error_new();
    self->state = DECODER_STATE_ERROR;
    return;
  }

  /*self->year =*/ut_uint8_list_get_uint16_be(data, offset + 0);
  /*self->month =*/ut_uint8_list_get_element(data, offset + 2);
  /*self->day =*/ut_uint8_list_get_element(data, offset + 3);
  /*self->hour =*/ut_uint8_list_get_element(data, offset + 4);
  /*self->minute =*/ut_uint8_list_get_element(data, offset + 5);
  /*self->second =*/ut_uint8_list_get_element(data, offset + 6);
}

static size_t decode_chunk(UtPngDecoder *self, UtObject *data, size_t offset) {
  size_t data_length = ut_list_get_length(data) - offset;

  // Minimum chunk is 12 bytes (length + type + CRC).
  if (data_length < 12) {
    return 0;
  }

  uint32_t block_data_length = ut_uint8_list_get_uint32_be(data, offset + 0);
  size_t total_block_length = 12 + block_data_length;
  if (data_length < total_block_length) {
    return 0;
  }

  ChunkType type = ut_uint8_list_get_uint32_be(data, offset + 4);

  size_t block_data_offset = offset + 8;
  switch (type) {
  case IMAGE_HEADER:
    decode_image_header(self, data, block_data_offset, block_data_length);
    break;
  case PALETTE:
    decode_palette(self, data, block_data_offset, block_data_length);
    break;
  case IMAGE_DATA:
    decode_image_data(self, data, block_data_offset, block_data_length);
    break;
  case CHROMATICITIES:
    break;
  case GAMMA:
    break;
  case ICC_PROFILE:
    break;
  case SIGNIFICANT_BITS:
    break;
  case STANDARD_RGB:
    break;
  case IMAGE_END:
    decode_image_end(self, data, block_data_offset, block_data_length);
    break;
  case BACKGROUND:
    decode_background(self, data, block_data_offset, block_data_length);
    break;
  case HISTOGRAM:
    break;
  case PHYSICAL_DIMENSIONS:
    decode_physical_dimensions(self, data, block_data_offset,
                               block_data_length);
    break;
  case SUGGESTED_PALETTE:
    break;
  case MODIFICATION_TIME:
    decode_modification_time(self, data, block_data_offset, block_data_length);
    break;
  case TEXT:
    break;
  case COMPRESSED_TEXT:
    break;
  case INTERNATIONAL_TEXT:
    break;
  }

  return total_block_length;
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtPngDecoder *self = user_data;

  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->read_cancel);
    return 0;
  }

  size_t offset = 0;
  while (true) {
    size_t n_used;
    switch (self->state) {
    case DECODER_STATE_SIGNATURE:
      n_used = decode_signature(self, data, offset);
      break;
    case DECODER_STATE_CHUNK:
      n_used = decode_chunk(self, data, offset);
      break;
    case DECODER_STATE_ERROR:
      ut_cancel_activate(self->read_cancel);
      self->callback(self->user_data, self->error);
      return offset;
    case DECODER_STATE_END:
      ut_cancel_activate(self->read_cancel);
      UtObjectRef image = ut_png_image_new(self->width, self->height,
                                           self->bit_depth, self->colour_type);
      self->callback(self->user_data, image);
      return offset;
    default:
      assert(false);
    }

    if (n_used == 0) {
      return offset;
    }

    offset += n_used;
  }
}

static void sync_cb(void *user_data, UtObject *image) {
  UtObject **result = user_data;
  assert(*result == NULL);
  *result = ut_object_ref(image);
}

static void ut_png_decoder_init(UtObject *object) {
  UtPngDecoder *self = (UtPngDecoder *)object;
  self->input_stream = NULL;
  self->read_cancel = ut_cancel_new();
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
  self->state = DECODER_STATE_SIGNATURE;
  self->error = NULL;
  self->width = 0;
  self->height = 0;
  self->bit_depth = 0;
  self->colour_type = 0;
  self->compression_method = 0;
  self->filter_method = 0;
  self->interlace_method = 0;
}

static void ut_png_decoder_cleanup(UtObject *object) {
  UtPngDecoder *self = (UtPngDecoder *)object;
  ut_cancel_activate(self->read_cancel);
  ut_object_unref(self->input_stream);
  ut_object_unref(self->read_cancel);
  ut_object_unref(self->cancel);
  ut_object_unref(self->error);
}

static UtObjectInterface object_interface = {.type_name = "UtPngDecoder",
                                             .init = ut_png_decoder_init,
                                             .cleanup = ut_png_decoder_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_png_decoder_new(UtObject *input_stream) {
  UtObject *object = ut_object_new(sizeof(UtPngDecoder), &object_interface);
  UtPngDecoder *self = (UtPngDecoder *)object;
  self->input_stream = ut_object_ref(input_stream);
  return object;
}

void ut_png_decoder_decode(UtObject *object, UtPngDecodeCallback callback,
                           void *user_data, UtObject *cancel) {
  assert(ut_object_is_png_decoder(object));
  UtPngDecoder *self = (UtPngDecoder *)object;

  assert(self->callback == NULL);
  assert(callback != NULL);

  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);

  ut_input_stream_read(self->input_stream, read_cb, self, self->read_cancel);
}

UtObject *ut_png_decoder_decode_sync(UtObject *object) {
  UtObject *result = NULL;
  UtObjectRef cancel = ut_cancel_new();
  ut_png_decoder_decode(object, sync_cb, &result, cancel);
  ut_cancel_activate(cancel);
  if (result == NULL) {
    result = ut_general_error_new("Sync call did not complete");
  }
  return result;
}

bool ut_object_is_png_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
