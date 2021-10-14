#include <assert.h>
#include <string.h>

#include "ut-mutable-uint32-list.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-utf8-string.h"

int ut_utf8_string_id = 0;

const char *ut_utf8_string_get_text(UtObject *object) {
  UtUtf8StringFunctions *utf8_string_functions =
      ut_object_get_interface(object, &ut_utf8_string_id);
  assert(utf8_string_functions != NULL);
  return utf8_string_functions->get_text(object);
}

UtObject *ut_utf8_string_get_code_points(UtObject *object) {
  const char *text = ut_utf8_string_get_text(object);
  size_t text_length = strlen(text);
  UtObject *code_points = ut_mutable_uint32_list_new();
  size_t offset = 0;
  while (offset < text_length) {
    uint8_t byte1 = text[offset];
    if ((byte1 & 0x80) == 0) {
      ut_mutable_uint32_list_append(code_points, byte1);
      offset++;
    } else if ((byte1 & 0xe0) == 0xc0) {
      if (text_length - offset < 2) {
        return code_points;
      }
      uint8_t byte2 = text[offset + 1];
      if ((byte2 & 0xc0) != 0x80) {
        return code_points;
      }
      ut_mutable_uint32_list_append(code_points,
                                    (byte1 & 0x1f) << 6 | (byte2 & 0x3f));
      offset += 2;
    } else if ((byte1 & 0xf0) == 0xe0) {
      if (text_length - offset < 3) {
        return code_points;
      }
      uint8_t byte2 = text[offset + 1];
      uint8_t byte3 = text[offset + 2];
      if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80) {
        return code_points;
      }
      ut_mutable_uint32_list_append(code_points, (byte1 & 0x0f) << 12 |
                                                     (byte2 & 0x3f) << 6 |
                                                     (byte3 & 0x3f));
      offset += 3;
    } else if ((byte1 & 0xf8) == 0xf0) {
      if (text_length - offset < 4) {
        return code_points;
      }
      uint8_t byte2 = text[offset + 1];
      uint8_t byte3 = text[offset + 2];
      uint8_t byte4 = text[offset + 3];
      if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80 ||
          (byte4 & 0xc0) != 0x80) {
        return code_points;
      }
      ut_mutable_uint32_list_append(
          code_points, (byte1 & 0x07) << 18 | (byte2 & 0x3f) << 12 |
                           (byte3 & 0x3f) << 6 | (byte4 & 0x3f));
      offset += 4;
    } else {
      return code_points;
    }
  }

  return code_points;
}

bool ut_object_implements_utf8_string(UtObject *object) {
  return ut_object_get_interface(object, &ut_utf8_string_id) != NULL;
}
