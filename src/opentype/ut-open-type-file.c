#include <assert.h>

#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-general-error.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-open-type-file.h"
#include "ut-string.h"
#include "ut-uint16-list.h"
#include "ut-uint8-list.h"

#include <stdio.h>

#define TABLE_CHARACTER_TO_GLYPH_INDEX_MAPPING 0x636d6170
#define TABLE_CONTROL_VALUE 0x63767420
#define TABLE_CONTROL_VALUE_PROGRAM 0x70726570
#define TABLE_DIGITAL_SIGNATURE 0x44534947
#define TABLE_FONT_HEADER 0x68656164
#define TABLE_FONT_PROGRAM 0x6670676d
#define TABLE_GLYPH_DATA 0x676c7966
#define TABLE_GLYPH_POSITIONING 0x47504f53
#define TABLE_GLYPH_SUBSTITUTION 0x47535542
#define TABLE_GRID_FITTING_AND_SCAN_CONVERSION_PROCEDURE 0x67617370
#define TABLE_HORIZONTAL_DEVICE_METRICS 0x68646d78
#define TABLE_INDEX_TO_LOCATION 0x6c6f6361
#define TABLE_KERNING 0x6b65726e
#define TABLE_LINEAR_THRESHOLD 0x4c545348
#define TABLE_MAXIMUM_PROFILE 0x6d617870
#define TABLE_NAMING 0x6e616d65
#define TABLE_OS_2 0x4f532f32
#define TABLE_POSTSCRIPT 0x706f7374
#define TABLE_VERTICAL_DEVICE_METRICS 0x56444d58

typedef enum {
  PLATFORM_ID_UNICODE = 0,
  PLATFORM_ID_MACINTOSH = 1,
  PLATFORM_ID_WINDOWS = 3,
} PlatformId;

typedef enum {
  UNICODE_ENCODING_ID_UNICODE_1_0 = 0,
  UNICODE_ENCODING_ID_UNICODE_1_1 = 1,
  UNICODE_ENCODING_ID_ISO646 = 2,
  UNICODE_ENCODING_ID_UNICODE_2_0_BMP = 3,
  UNICODE_ENCODING_ID_UNICODE_2_0 = 4,
} UnicodeEncodingId;

typedef enum {
  NAME_ID_COPYRIGHT_NOTICE = 0,
  NAME_ID_FONT_FAMILY_NAME = 1,
  NAME_ID_FONT_SUBFAMILY_NAME = 2,
  NAME_ID_UNIQUE_FONT_IDENTIFIER = 3,
  NAME_ID_FULL_FONT_NAME = 4,
  NAME_ID_VERSION_STRING = 5,
  NAME_ID_POSTSCRIPT_NAME = 6,
  NAME_ID_TRADEMARK = 7,
  NAME_ID_MANUFACTURER_NAME = 8,
  NAME_ID_DESIGNER = 9,
  NAME_ID_DESCRIPTION = 10,
  NAME_ID_VENDOR_URL = 11,
  NAME_ID_DESIGNER_URL = 12,
  NAME_ID_LICENSE_DESCRIPTION = 13,
  NAME_ID_LICENSE_INFO_URL = 14,
  NAME_ID_TYPOGRAPHIC_FAMILY_NAME = 16,
  NAME_ID_TYPOGRAPHIC_SUBFAMILY_NAME = 17,
  NAME_ID_COMPATIBLE_FULL_NAME = 18,
  NAME_ID_SAMPLE_TEXT = 19,
} NameId;

#define GLYPH_FLAG_ON_CURVE_POINT 0x01
#define GLYPH_FLAG_X_SHORT_VECTOR 0x02
#define GLYPH_FLAG_Y_SHORT_VECTOR 0x04
#define GLYPH_FLAG_REPEAT_FLAG 0x08
#define GLYPH_FLAG_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR 0x10
#define GLYPH_FLAG_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR 0x20
#define GLYPH_FLAG_OVERLAP_SIMPLE 0x40

typedef struct {
  UtObject object;

  // The OpenType file.
  UtObject *data;

  uint16_t n_glyphs;
  uint16_t units_per_em;
  uint16_t index_to_loc_format;
} UtOpenTypeFile;

static uint8_t get_uint8(UtOpenTypeFile *self, size_t offset) {
  return ut_uint8_list_get_element(self->data, offset);
}

static uint16_t get_uint16(UtOpenTypeFile *self, size_t offset) {
  return ut_uint8_list_get_uint16_be(self->data, offset);
}

static int16_t get_int16(UtOpenTypeFile *self, size_t offset) {
  return ut_uint8_list_get_int16_be(self->data, offset);
}

static uint32_t get_uint32(UtOpenTypeFile *self, size_t offset) {
  return ut_uint8_list_get_uint32_be(self->data, offset);
}

static char *get_utf16_string(UtOpenTypeFile *self, size_t offset,
                              size_t length) {
  UtObjectRef code_units = ut_uint16_list_new();
  for (size_t i = 0; i < length / 2; i++) {
    ut_uint16_list_append(code_units, get_uint16(self, offset + 2 * i));
  }
  UtObjectRef string = ut_string_new_from_utf16(code_units);
  return ut_string_take_text(string);
}

static bool read_table_directory(UtOpenTypeFile *self) {
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

  return true;
}

static bool get_table(UtOpenTypeFile *self, uint32_t tag, size_t *offset,
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

static void read_font_header(UtOpenTypeFile *self) {
  size_t table_offset, length;
  assert(get_table(self, TABLE_FONT_HEADER, &table_offset, &length));

  size_t o = table_offset;
  assert(length >= 54);
  uint16_t major_version = get_uint16(self, o);
  assert(major_version == 1);
  uint16_t minor_version = get_uint16(self, o + 2);
  assert(minor_version == 0);
  // font revision
  // checksum adjustment
  uint32_t magic_number = get_uint32(self, o + 12);
  assert(magic_number == 0x5F0F3CF5);
  // flags
  self->units_per_em = get_uint16(self, o + 18);
  // created
  // modified
  // xmin
  // ymin
  // xmax
  // ymax
  // mac_style
  // lowest_rec_ppem
  // font_direction_hint (deprecated)
  self->index_to_loc_format = get_uint16(self, o + 50);
  uint16_t glyph_data_format = get_uint16(self, o + 52);
  assert(glyph_data_format == 0);
}

static void read_maximum_profile(UtOpenTypeFile *self) {
  size_t table_offset, length;
  assert(get_table(self, TABLE_MAXIMUM_PROFILE, &table_offset, &length));

  size_t o = table_offset;
  assert(length >= 4);
  uint32_t version = get_uint32(self, o);
  assert(version == 0x00005000 || version == 0x00010000);
  self->n_glyphs = get_uint16(self, o + 4);
}

static char *get_string(UtOpenTypeFile *self, uint16_t name_id) {
  size_t table_offset, length;
  assert(get_table(self, TABLE_NAMING, &table_offset, &length));

  size_t o = table_offset;
  assert(length >= 6);
  uint16_t version = get_uint16(self, o);
  assert(version == 0 || version == 1);
  uint16_t count = get_uint16(self, o + 2);
  size_t storage_offset = table_offset + get_uint16(self, o + 4);
  size_t storage_length = length - storage_offset;
  assert(length >= 6 + 12 * count);
  o += 6;
  for (size_t i = 0; i < count; i++) {
    uint16_t platform_id = get_uint16(self, o);
    uint16_t encoding_id = get_uint16(self, o + 2);
    // uint16_t language_id = get_uint16(self, o + 4);
    uint16_t name_id_ = get_uint16(self, o + 6);
    if (name_id_ == name_id) {
      size_t string_length = get_uint16(self, o + 8);
      size_t string_offset = storage_offset + get_uint16(self, o + 10);

      if (platform_id == PLATFORM_ID_WINDOWS && encoding_id == 1) {
        assert(string_offset <= storage_length);
        assert(string_offset + string_length <= storage_length);
        return get_utf16_string(self, string_offset, string_length);
      }
    }

    o += 12;
  }

  return NULL;
}

static bool get_character_map(UtOpenTypeFile *self, uint16_t platform_id,
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

static bool get_unicode_glyph_index(UtOpenTypeFile *self, uint32_t code_point,
                                    uint16_t *index) {
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
  size_t glyph_id_array_offset = o;

  // FIXME: Use binary search
  for (size_t i = 0; i < segment_count; i++) {
    uint16_t end_code = get_uint16(self, end_code_offset + i * 2);
    if (end_code < code_point) {
      continue;
    }

    uint16_t start_code = get_uint16(self, start_code_offset + i * 2);
    if (start_code > code_point) {
      return false;
    }

    int16_t id_delta = get_int16(self, id_delta_offset + i * 2);
    uint16_t id_range_offset =
        get_uint16(self, id_range_offsets_offset + i * 2);
    uint16_t glyph_index =
        id_range_offset != 0
            ? get_int16(self, glyph_id_array_offset + id_range_offset * 2)
            : 0;

    *index = code_point + id_delta + glyph_index;
    return true;
  }

  return false;
}

static bool get_glyph_offset(UtOpenTypeFile *self, uint16_t index,
                             size_t *offset) {
  size_t table_offset, length;
  assert(get_table(self, TABLE_INDEX_TO_LOCATION, &table_offset, &length));

  if (index >= self->n_glyphs) {
    return false;
  }

  if (self->index_to_loc_format == 0) {
    *offset = (size_t)get_uint16(self, table_offset + index * 2) * 2;
    return true;
  } else if (self->index_to_loc_format == 1) {
    *offset = get_uint32(self, table_offset + index * 4);
    return true;
  } else {
    assert(false);
  }
}

static bool get_glyph(UtOpenTypeFile *self, uint16_t index) {
  size_t table_offset, length;
  assert(get_table(self, TABLE_GLYPH_DATA, &table_offset, &length));
  size_t end = table_offset + length;

  size_t glyph_offset;
  if (!get_glyph_offset(self, index, &glyph_offset)) {
    return false;
  }
  assert(glyph_offset < length);

  size_t header_offset = table_offset + glyph_offset;
  size_t header_length = 10;
  assert(header_offset + header_length <= end);

  int16_t n_contours = get_int16(self, header_offset);
  int16_t x_min = get_int16(self, header_offset + 2);
  int16_t y_min = get_int16(self, header_offset + 4);
  int16_t x_max = get_int16(self, header_offset + 6);
  int16_t y_max = get_int16(self, header_offset + 8);
  printf("%d  x:%d-%d y:%d-%d\n", n_contours, x_min, x_max, y_min, y_max);

  assert(n_contours >= 0);
  size_t end_points_offset = header_offset + header_length;
  size_t end_points_length = 2 * n_contours;
  assert(+end_points_length <= end);
  // end_points
  size_t n_points = get_uint16(self, end_points_offset + 2 * (n_contours - 1));
  printf("%zi\n", n_points);

  size_t instructions_header_offset = end_points_offset + end_points_length;
  size_t instructions_header_length = 2;
  assert(instructions_header_offset + instructions_header_length <= end);
  uint16_t instructions_length = get_uint16(self, instructions_header_offset);
  size_t instructions_offset =
      instructions_header_offset + instructions_header_length;
  assert(instructions_offset + instructions_length <= end);
  UtObjectRef instructions =
      ut_list_get_sublist(self->data, instructions_offset, instructions_length);
  printf("instructions %s\n", ut_object_to_string(instructions));

  // Calculate variable lengths of flags, x and y co-ordinates.
  size_t flags_offset = instructions_offset + instructions_length;
  size_t flag_offset = 0;
  size_t x_coords_length = 0;
  size_t y_coords_length = 0;
  for (size_t i = 0; i < n_points; i++) {
    uint8_t flag = get_uint8(self, flags_offset + flag_offset);
    flag_offset++;

    size_t x_coord_length = 2;
    if ((flag & GLYPH_FLAG_X_SHORT_VECTOR) != 0) {
      x_coord_length = 1;
    } else if ((flag & GLYPH_FLAG_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) != 0) {
      x_coord_length = 0;
    }
    size_t y_coord_length = 2;
    if ((flag & GLYPH_FLAG_Y_SHORT_VECTOR) != 0) {
      y_coord_length = 1;
    } else if ((flag & GLYPH_FLAG_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) != 0) {
      y_coord_length = 0;
    }

    uint8_t repeat_count = 0;
    if ((flag & GLYPH_FLAG_REPEAT_FLAG) != 0) {
      repeat_count = get_uint8(self, flags_offset + flag_offset);
      flag_offset++;
      i += repeat_count;
      assert(i < n_points);
    }

    x_coords_length += x_coord_length * (1 + repeat_count);
    y_coords_length += y_coord_length * (1 + repeat_count);
  }
  size_t flags_length = flag_offset;

  assert(flags_offset + flags_length + x_coords_length + y_coords_length <=
         end);
  size_t x_coords_offset = flags_offset + flags_length;
  size_t y_coords_offset = x_coords_offset + x_coords_length;

  int16_t x = 0, y = 0;
  flag_offset = 0;
  size_t xo = x_coords_offset;
  size_t yo = y_coords_offset;
  for (size_t i = 0; i < n_points; i++) {
    uint8_t flag = get_uint8(self, flags_offset + flag_offset);
    flag_offset++;

    uint8_t repeat_count = 0;
    if ((flag & GLYPH_FLAG_REPEAT_FLAG) != 0) {
      repeat_count = get_uint8(self, flags_offset + flag_offset);
      flag_offset++;
      i += repeat_count;
    }

    for (size_t j = 0; j <= repeat_count; j++) {
      int16_t dx = 0;
      if ((flag & GLYPH_FLAG_X_SHORT_VECTOR) != 0) {
        dx = get_uint8(self, xo);
        xo++;
        if ((flag & GLYPH_FLAG_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) == 0) {
          dx = -dx;
        }
      } else if ((flag & GLYPH_FLAG_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) ==
                 0) {
        dx = get_int16(self, xo);
        xo += 2;
      }
      x += dx;

      int16_t dy = 0;
      if ((flag & GLYPH_FLAG_Y_SHORT_VECTOR) != 0) {
        dy = get_uint8(self, yo);
        yo++;
        if ((flag & GLYPH_FLAG_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) == 0) {
          dy = -dy;
        }
      } else if ((flag & GLYPH_FLAG_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) ==
                 0) {
        dy = get_int16(self, yo);
        yo += 2;
      }
      y += dy;

      printf("%02x %d %d (%d, %d)\n", flag, x, y, dx, dy);
    }
  }

  return true;
}

static void ut_open_type_decoder_init(UtObject *object) {
  UtOpenTypeFile *self = (UtOpenTypeFile *)object;
  self->data = NULL;
  self->n_glyphs = 0;
}

static void ut_open_type_decoder_cleanup(UtObject *object) {
  UtOpenTypeFile *self = (UtOpenTypeFile *)object;
  ut_object_unref(self->data);
}

static UtObjectInterface object_interface = {.type_name = "UtOpenTypeFile",
                                             .init = ut_open_type_decoder_init,
                                             .cleanup =
                                                 ut_open_type_decoder_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_open_type_file_new(UtObject *data) {
  UtObject *object = ut_object_new(sizeof(UtOpenTypeFile), &object_interface);
  UtOpenTypeFile *self = (UtOpenTypeFile *)object;
  self->data = ut_object_ref(data);
  return object;
}

void ut_open_type_file_open(UtObject *object) {
  assert(ut_object_is_open_type_file(object));
  UtOpenTypeFile *self = (UtOpenTypeFile *)object;

  read_table_directory(self);
  read_font_header(self);
  read_maximum_profile(self);

  ut_cstring_ref family = get_string(self, NAME_ID_FONT_FAMILY_NAME);
  ut_cstring_ref subfamily = get_string(self, NAME_ID_FONT_SUBFAMILY_NAME);
  printf("%s %s\n", family, subfamily);

  uint32_t code_point = 65;
  uint16_t index;
  if (get_unicode_glyph_index(self, code_point, &index)) {
    printf("code point %d -> glyph %d\n", code_point, index);
    get_glyph(self, index);
  }
}

bool ut_object_is_open_type_file(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
