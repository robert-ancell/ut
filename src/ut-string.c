#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ut-constant-utf8-string.h"
#include "ut-general-error.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-uint16-list.h"
#include "ut-uint32-list.h"
#include "ut-utf8-string.h"

int ut_string_id = 0;

UtObject *ut_string_new(const char *text) { return ut_utf8_string_new(text); }

UtObject *ut_string_new_constant(const char *text) {
  return ut_constant_utf8_string_new(text);
}

UtObject *ut_string_new_sized(const char *text, size_t length) {
  return ut_utf8_string_new_sized(text, length);
}

UtObject *ut_string_new_from_utf8(UtObject *utf8) {
  return ut_utf8_string_new_from_data(utf8);
}

const char *ut_string_get_text(UtObject *object) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  return string_interface->get_text(object);
}

char *ut_string_take_text(UtObject *object) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  if (string_interface->take_text != NULL) {
    return string_interface->take_text(object);
  }

  return strdup(ut_string_get_text(object));
}

UtObject *ut_string_get_code_points(UtObject *object) {
  const char *text = ut_string_get_text(object);
  size_t text_length = strlen(text);
  UtObjectRef code_points = ut_uint32_list_new();
  size_t offset = 0;
  while (offset < text_length) {
    uint8_t byte1 = text[offset];
    if ((byte1 & 0x80) == 0) {
      ut_uint32_list_append(code_points, byte1);
      offset++;
    } else if ((byte1 & 0xe0) == 0xc0) {
      if (text_length - offset < 2) {
        return ut_general_error_new("Invalid UTF-8");
      }
      uint8_t byte2 = text[offset + 1];
      if ((byte2 & 0xc0) != 0x80) {
        return ut_general_error_new("Invalid UTF-8");
      }
      ut_uint32_list_append(code_points, (byte1 & 0x1f) << 6 | (byte2 & 0x3f));
      offset += 2;
    } else if ((byte1 & 0xf0) == 0xe0) {
      if (text_length - offset < 3) {
        return ut_general_error_new("Invalid UTF-8");
      }
      uint8_t byte2 = text[offset + 1];
      uint8_t byte3 = text[offset + 2];
      if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80) {
        return ut_general_error_new("Invalid UTF-8");
      }
      ut_uint32_list_append(code_points, (byte1 & 0x0f) << 12 |
                                             (byte2 & 0x3f) << 6 |
                                             (byte3 & 0x3f));
      offset += 3;
    } else if ((byte1 & 0xf8) == 0xf0) {
      if (text_length - offset < 4) {
        return ut_general_error_new("Invalid UTF-8");
      }
      uint8_t byte2 = text[offset + 1];
      uint8_t byte3 = text[offset + 2];
      uint8_t byte4 = text[offset + 3];
      if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80 ||
          (byte4 & 0xc0) != 0x80) {
        return ut_general_error_new("Invalid UTF-8");
      }
      ut_uint32_list_append(code_points,
                            (byte1 & 0x07) << 18 | (byte2 & 0x3f) << 12 |
                                (byte3 & 0x3f) << 6 | (byte4 & 0x3f));
      offset += 4;
    } else {
      return ut_general_error_new("Invalid UTF-8");
    }
  }

  return ut_object_ref(code_points);
}

UtObject *ut_string_get_utf8(UtObject *object) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  return string_interface->get_utf8(object);
}

UtObject *ut_string_get_utf16(UtObject *object) {
  UtObjectRef code_points = ut_string_get_code_points(object);
  UtObjectRef utf16 = ut_uint16_list_new();
  size_t code_points_length = ut_list_get_length(code_points);
  for (size_t i = 0; i < code_points_length; i++) {
    uint32_t code_point = ut_uint32_list_get_element(code_points, i);
    if (code_point <= 0xffff) {
      if (code_point >= 0xd800 && code_point <= 0xdfff) {
        return ut_general_error_new("Invalid code points");
      }
      ut_uint16_list_append(utf16, code_point);
    } else if (code_point <= 0x10ffff) {
      uint32_t u = code_point - 0x10000;
      ut_uint16_list_append(utf16, 0xd800 + (u >> 10));
      ut_uint16_list_append(utf16, 0xdc00 + (u & 0x3ff));
    } else {
      return ut_general_error_new("Invalid code points");
    }
  }

  return ut_object_ref(utf16);
}

bool ut_string_is_mutable(UtObject *object) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  return string_interface->is_mutable;
}

void ut_string_clear(UtObject *object) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  assert(string_interface->is_mutable);
  string_interface->clear(object);
}

void ut_string_prepend(UtObject *object, const char *text) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  assert(string_interface->is_mutable);
  string_interface->prepend(object, text);
}

void ut_string_prepend_code_point(UtObject *object, uint32_t code_point) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  assert(string_interface->is_mutable);
  string_interface->prepend_code_point(object, code_point);
}

void ut_string_append(UtObject *object, const char *text) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  assert(string_interface->is_mutable);
  string_interface->append(object, text);
}

void ut_string_append_code_point(UtObject *object, uint32_t code_point) {
  UtStringInterface *string_interface =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_interface != NULL);
  assert(string_interface->is_mutable);
  string_interface->append_code_point(object, code_point);
}

char *ut_string_to_string(UtObject *object) {
  UtObjectRef string = ut_string_new("\"");
  for (const char *c = ut_string_get_text(object); *c != '\0'; c++) {
    if (*c == 0x7) {
      ut_string_append(string, "\\a");
    } else if (*c == 0x8) {
      ut_string_append(string, "\\b");
    } else if (*c == 0x9) {
      ut_string_append(string, "\\t");
    } else if (*c == 0xa) {
      ut_string_append(string, "\\n");
    } else if (*c == 0xb) {
      ut_string_append(string, "\\v");
    } else if (*c == 0xc) {
      ut_string_append(string, "\\f");
    } else if (*c == 0xd) {
      ut_string_append(string, "\\r");
    } else if (*c == 0x1b) {
      ut_string_append(string, "\\e");
    } else if (*c == 0x22) {
      ut_string_append(string, "\\\"");
    } else if (*c == 0x5c) {
      ut_string_append(string, "\\\\");
    } else if (*c == 0x7f || *c <= 0x1f) {
      ut_string_append(string, "\\x");
      char hex_value[3];
      snprintf(hex_value, 3, "%02x", *c);
      ut_string_append(string, hex_value);
    } else {
      ut_string_append_code_point(string, *c);
    }
  }
  ut_string_append(string, "\"");

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
