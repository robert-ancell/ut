#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ut-mutable-string.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint32-array.h"

int ut_string_id = 0;

UtObject *ut_string_new(const char *text) {
  return ut_mutable_string_new(text);
}

const char *ut_string_get_text(UtObject *object) {
  UtStringFunctions *string_functions =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_functions != NULL);
  return string_functions->get_text(object);
}

char *ut_string_take_text(UtObject *object) {
  UtStringFunctions *string_functions =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_functions != NULL);
  if (string_functions->take_text != NULL) {
    return string_functions->take_text(object);
  }

  return strdup(ut_string_get_text(object));
}

UtObject *ut_string_get_code_points(UtObject *object) {
  const char *text = ut_string_get_text(object);
  size_t text_length = strlen(text);
  UtObject *code_points = ut_uint32_array_new();
  size_t offset = 0;
  while (offset < text_length) {
    uint8_t byte1 = text[offset];
    if ((byte1 & 0x80) == 0) {
      ut_uint32_array_append(code_points, byte1);
      offset++;
    } else if ((byte1 & 0xe0) == 0xc0) {
      if (text_length - offset < 2) {
        return code_points;
      }
      uint8_t byte2 = text[offset + 1];
      if ((byte2 & 0xc0) != 0x80) {
        return code_points;
      }
      ut_uint32_array_append(code_points, (byte1 & 0x1f) << 6 | (byte2 & 0x3f));
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
      ut_uint32_array_append(code_points, (byte1 & 0x0f) << 12 |
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
      ut_uint32_array_append(code_points,
                             (byte1 & 0x07) << 18 | (byte2 & 0x3f) << 12 |
                                 (byte3 & 0x3f) << 6 | (byte4 & 0x3f));
      offset += 4;
    } else {
      return code_points;
    }
  }

  return code_points;
}

char *ut_string_to_string(UtObject *object) {
  UtObjectRef string = ut_mutable_string_new("\"");
  for (const char *c = ut_string_get_text(object); *c != '\0'; c++) {
    if (*c == 0x7) {
      ut_mutable_string_append(string, "\\a");
    } else if (*c == 0x8) {
      ut_mutable_string_append(string, "\\b");
    } else if (*c == 0x9) {
      ut_mutable_string_append(string, "\\t");
    } else if (*c == 0xa) {
      ut_mutable_string_append(string, "\\n");
    } else if (*c == 0xb) {
      ut_mutable_string_append(string, "\\v");
    } else if (*c == 0xc) {
      ut_mutable_string_append(string, "\\f");
    } else if (*c == 0xd) {
      ut_mutable_string_append(string, "\\r");
    } else if (*c == 0x1b) {
      ut_mutable_string_append(string, "\\e");
    } else if (*c == 0x22) {
      ut_mutable_string_append(string, "\\\"");
    } else if (*c == 0x5c) {
      ut_mutable_string_append(string, "\\\\");
    } else if (*c == 0x7f || *c <= 0x1f) {
      ut_mutable_string_append(string, "\\x");
      char hex_value[3];
      snprintf(hex_value, 3, "%02x", *c);
      ut_mutable_string_append(string, hex_value);
    } else {
      ut_mutable_string_append_code_point(string, *c);
    }
  }
  ut_mutable_string_append(string, "\"");

  return ut_string_take_text(string);
}

bool ut_string_equal(UtObject *object, UtObject *other) {
  if (!ut_object_implements_string(other)) {
    return false;
  }
  return strcmp(ut_string_get_text(object), ut_string_get_text(other)) == 0;
}

int ut_string_hash(UtObject *object) {
  int hash = 0;
  for (const char *c = ut_string_get_text(object); *c != '\0'; c++) {
    hash = hash * 31 + *c;
  }
  return hash;
}

bool ut_object_implements_string(UtObject *object) {
  return ut_object_get_interface(object, &ut_string_id) != NULL;
}
