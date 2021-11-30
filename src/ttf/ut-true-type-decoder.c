#include <assert.h>

#include "ut-cancel.h"
#include "ut-general-error.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-true-type-decoder.h"
#include "ut-true-type-font.h"
#include "ut-uint8-list.h"

#include <stdio.h>

#define TABLE_FONT_HEADER 0x68656164
#define TABLE_CHARACTER_TO_GLYPH_INDEX_MAPPING 0x636d6170
#define TABLE_GLYPH_DATA 0x676c7966
#define TABLE_GLYPH_POSITIONING 0x47504f53
#define TABLE_GLYPH_SUBSTITUTION 0x47535542
#define TABLE_VERTICAL_DEVICE_METRICS 0x56444d58
#define TABLE_CONTROL_VALUE 0x63767420

typedef struct {
  UtObject object;
  UtObject *data;
} UtTrueTypeDecoder;

static uint16_t get_uint16(UtTrueTypeDecoder *self, size_t offset) {
  return ut_uint8_list_get_uint16_be(self->data, offset);
}

static int16_t get_int16(UtTrueTypeDecoder *self, size_t offset) {
  return ut_uint8_list_get_int16_be(self->data, offset);
}

static uint32_t get_uint32(UtTrueTypeDecoder *self, size_t offset) {
  return ut_uint8_list_get_uint32_be(self->data, offset);
}

static bool read_table_directory(UtTrueTypeDecoder *self) {
  size_t data_length = ut_list_get_length(self->data);
  if (data_length < 12) {
    return false;
  }

  uint32_t sfnt_version = get_uint32(self, 0);
  if (sfnt_version != 0x00010000) {
    return false;
  }

  uint16_t n_tables = get_uint16(self, 4);
  // search_range = get_uint16(self, 6);
  // entry_selector = get_uint16(self, 8);
  // range_shift = get_uint16(self, 10);

  size_t directory_length = 12 + n_tables * 16;
  if (data_length < directory_length) {
    return false;
  }

  size_t offset = 12;
  for (size_t i = 0; i < n_tables; i++) {
    uint32_t tag = get_uint32(self, offset);
    // uint32_t checksum = get_uint32(self, offset + 4);
    uint32_t table_offset = get_uint32(self, offset + 8);
    uint32_t table_length = get_uint32(self, offset + 12);
    printf("%c%c%c%c %08x %.10d %d\n", tag >> 24, (tag >> 16) & 0xff,
           (tag >> 8) & 0xff, tag & 0xff, tag, table_offset, table_length);
    offset += 16;
  }

  return true;
}

static bool get_table(UtTrueTypeDecoder *self, uint32_t tag, size_t *offset,
                      size_t *length) {
  uint16_t n_tables = get_uint16(self, 4);

  size_t o = 12;
  for (size_t i = 0; i < n_tables; i++) {
    uint32_t t = get_uint32(self, o);
    if (t == tag) {
      *offset = get_uint32(self, o + 8);
      *length = get_uint32(self, o + 12);
      return true;
    }
    o += 16;
  }

  return false;
}

static bool get_character_map(UtTrueTypeDecoder *self, uint16_t platform_id,
                              uint16_t *encoding_id, size_t *offset) {
  size_t table_offset, length;
  assert(get_table(self, TABLE_CHARACTER_TO_GLYPH_INDEX_MAPPING, &table_offset,
                   &length));

  size_t o = table_offset;
  assert(length >= 4);
  uint16_t version = get_uint16(self, o);
  assert(version == 0);
  uint16_t n_tables = get_uint16(self, o + 2);
  assert(length >= 4 + n_tables * 8);
  o += 4;

  for (size_t i = 0; i < n_tables; i++) {
    if (get_uint16(self, o) == platform_id) {
      *encoding_id = get_uint16(self, o + 2);
      *offset = table_offset + get_uint32(self, o + 4);
      return true;
    }
    o += 8;
  }

  return false;
}

static bool get_unicode_character_map(UtTrueTypeDecoder *self) {
  uint16_t encoding_id;
  size_t offset;
  if (!get_character_map(self, 0, &encoding_id, &offset)) {
    return false;
  }

  size_t o = offset;
  uint16_t format = get_uint16(self, o);
  assert(format == 4);
  // uint16_t length = get_uint16(self, o+ 2);
  // uint16_t language = get_uint16(self, o+ 4);
  uint16_t segment_count_x2 = get_uint16(self, o + 6);
  assert(segment_count_x2 % 2 == 0);
  size_t segment_count = segment_count_x2 / 2;
  o += 14;
  size_t end_code_offset = o;
  o += segment_count * 2;
  o += 2;
  size_t start_code_offset = o;
  o += segment_count * 2;
  size_t id_delta_offset = o;
  o += segment_count * 2;
  size_t id_range_offsets_offset = o;
  o += segment_count * 2;
  // size_t glyph_id_array_offset = o;

  for (size_t i = 0; i < segment_count; i++) {
    uint16_t start_code = get_uint16(self, start_code_offset + i * 2);
    uint16_t end_code = get_uint16(self, end_code_offset + i * 2);
    int16_t id_delta = get_int16(self, id_delta_offset + i * 2);
    uint16_t id_range_offset =
        get_uint16(self, id_range_offsets_offset + i * 2);
    printf("%d-%d => %d-%d %d\n", start_code, end_code, start_code + id_delta,
           end_code + id_delta, id_range_offset);
  }

  return true;
}

static void ut_true_type_decoder_init(UtObject *object) {
  UtTrueTypeDecoder *self = (UtTrueTypeDecoder *)object;
  self->data = NULL;
}

static void ut_true_type_decoder_cleanup(UtObject *object) {
  UtTrueTypeDecoder *self = (UtTrueTypeDecoder *)object;
  ut_object_unref(self->data);
}

static UtObjectInterface object_interface = {.type_name = "UtTrueTypeDecoder",
                                             .init = ut_true_type_decoder_init,
                                             .cleanup =
                                                 ut_true_type_decoder_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_true_type_decoder_new(UtObject *data) {
  UtObject *object =
      ut_object_new(sizeof(UtTrueTypeDecoder), &object_interface);
  UtTrueTypeDecoder *self = (UtTrueTypeDecoder *)object;
  self->data = ut_object_ref(data);
  return object;
}

void ut_true_type_decoder_decode(UtObject *object) {
  assert(ut_object_is_true_type_decoder(object));
  UtTrueTypeDecoder *self = (UtTrueTypeDecoder *)object;

  read_table_directory(self);
  get_unicode_character_map(self);
}

bool ut_object_is_true_type_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
