#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ut-boolean.h"
#include "ut-float64.h"
#include "ut-hash-table.h"
#include "ut-int64.h"
#include "ut-json.h"
#include "ut-list.h"
#include "ut-map-item.h"
#include "ut-map.h"
#include "ut-mutable-list.h"
#include "ut-mutable-string.h"
#include "ut-null.h"
#include "ut-object-array.h"
#include "ut-string.h"

static bool encode_value(UtObject *buffer, UtObject *value);

static UtObject *decode_value(const char *text, size_t *offset);

static bool get_utf8_code_point(const char *text, size_t *offset,
                                uint32_t *code_point) {
  uint8_t byte1 = text[*offset];
  if ((byte1 & 0x80) == 0) {
    *code_point = byte1;
    (*offset)++;
    return true;
  } else if ((byte1 & 0xe0) == 0xc0) {
    uint8_t byte2;
    if ((byte2 = text[*offset + 1]) == '\0') {
      return false;
    }
    if ((byte2 & 0xc0) != 0x80) {
      // FIXME: Throw an error (invalid utf-8)
      return false;
    }
    *code_point = (byte1 & 0x1f) << 6 | (byte2 & 0x3f);
    (*offset) += 2;
    return true;
  } else if ((byte1 & 0xf0) == 0xe0) {
    uint8_t byte2, byte3;
    if ((byte2 = text[*offset + 1]) == '\0' ||
        (byte3 = text[*offset + 2]) == '\0') {
      return false;
    }
    if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80) {
      // FIXME: Throw an error (invalid utf-8)
      return false;
    }
    *code_point = (byte1 & 0x0f) << 12 | (byte2 & 0x3f) << 6 | (byte3 & 0x3f);
    (*offset) += 3;
    return true;
  } else if ((byte1 & 0xf8) == 0xf0) {
    uint8_t byte2, byte3, byte4;
    if ((byte2 = text[*offset + 1]) == '\0' ||
        (byte3 = text[*offset + 2]) == '\0' ||
        (byte4 = text[*offset + 3]) == '\0') {
      return false;
    }
    if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80 ||
        (byte4 & 0xc0) != 0x80) {
      // FIXME: Throw an error (invalid utf-8)
      return false;
    }
    *code_point = (byte1 & 0x07) << 18 | (byte2 & 0x3f) << 12 |
                  (byte3 & 0x3f) << 6 | (byte4 & 0x3f);
    (*offset) += 4;
    return true;
  } else {
    // FIXME: Throw an error (invalid utf-8)
    return false;
  }
}

static bool encode_string(UtObject *buffer, const char *value) {
  ut_mutable_string_append(buffer, "\"");
  size_t offset = 0;
  while (true) {
    uint32_t code_point;
    if (!get_utf8_code_point(value, &offset, &code_point)) {
      return false;
    }
    if (code_point == '\0') {
      ut_mutable_string_append(buffer, "\"");
      return true;
    }
    if (code_point <= 0x1f || code_point == 0x7f) {
      switch (code_point) {
      case '\b':
        ut_mutable_string_append(buffer, "\\b");
        break;
      case '\f':
        ut_mutable_string_append(buffer, "\\f");
        break;
      case '\n':
        ut_mutable_string_append(buffer, "\\n");
        break;
      case '\r':
        ut_mutable_string_append(buffer, "\\r");
        break;
      case '\t':
        ut_mutable_string_append(buffer, "\\t");
        break;
      default:
        char escape_sequence[7];
        snprintf(escape_sequence, 7, "\\u%04x", code_point);
        ut_mutable_string_append(buffer, escape_sequence);
        break;
      }
    } else if (code_point == '\"') {
      ut_mutable_string_append(buffer, "\\\"");
    } else if (code_point == '\\') {
      ut_mutable_string_append(buffer, "\\\\");
    } else {
      ut_mutable_string_append_code_point(buffer, code_point);
    }
  }
}

static bool encode_integer_number(UtObject *buffer, int64_t value) {
  char text[1024];
  snprintf(text, 1024, "%li", value);
  ut_mutable_string_append(buffer, text);
  return true;
}

static bool encode_float_number(UtObject *buffer, double value) {
  char text[1024];
  snprintf(text, 1024, "%f", value);

  // Strip trailing zeros.
  for (size_t i = strlen(text) - 1; i > 0; i--) {
    if (text[i] == '0') {
      text[i] = '\0';
    } else if (text[i] == '.') {
      text[i] = '\0';
      break;
    } else {
      break;
    }
  }

  ut_mutable_string_append(buffer, text);
  return true;
}

static bool encode_object(UtObject *buffer, UtObject *value) {
  ut_mutable_string_append(buffer, "{");
  UtObjectRef items = ut_map_get_items(value);
  size_t length = ut_list_get_length(items);
  for (size_t i = 0; i < length; i++) {
    UtObjectRef item = ut_list_get_element(items, i);
    if (i != 0) {
      ut_mutable_string_append(buffer, ",");
    }

    UtObjectRef key = ut_map_item_get_key(item);
    if (!ut_object_implements_string(key)) {
      // FIXME: Throw exception
      return false;
    }
    bool result = encode_string(buffer, ut_string_get_text(key));
    if (!result) {
      return false;
    }

    ut_mutable_string_append(buffer, ":");

    UtObjectRef value = ut_map_item_get_value(item);
    result = encode_value(buffer, value);
    if (!result) {
      return false;
    }
  }
  ut_mutable_string_append(buffer, "}");
  return true;
}

static bool encode_array(UtObject *buffer, UtObject *value) {
  ut_mutable_string_append(buffer, "[");
  size_t length = ut_list_get_length(value);
  for (size_t i = 0; i < length; i++) {
    UtObjectRef child = ut_list_get_element(value, i);
    if (i != 0) {
      ut_mutable_string_append(buffer, ",");
    }
    bool result = encode_value(buffer, child);
    if (!result) {
      return false;
    }
  }
  ut_mutable_string_append(buffer, "]");
  return true;
}

static bool encode_boolean(UtObject *buffer, bool value) {
  ut_mutable_string_append(buffer, value ? "true" : "false");
  return true;
}

static bool encode_null(UtObject *buffer) {
  ut_mutable_string_append(buffer, "null");
  return true;
}

static bool encode_value(UtObject *buffer, UtObject *value) {
  if (ut_object_implements_string(value)) {
    return encode_string(buffer, ut_string_get_text(value));
  } else if (ut_object_is_int64(value)) {
    return encode_integer_number(buffer, ut_int64_get_value(value));
  } else if (ut_object_is_float64(value)) {
    return encode_float_number(buffer, ut_float64_get_value(value));
  } else if (ut_object_implements_map(value)) {
    return encode_object(buffer, value);
  } else if (ut_object_implements_list(value)) {
    return encode_array(buffer, value);
  } else if (ut_object_is_boolean(value)) {
    return encode_boolean(buffer, ut_boolean_get_value(value));
  } else if (ut_object_is_null(value)) {
    return encode_null(buffer);
  } else {
    // FIXME: Throw error
    return false;
  }
}

static int decode_digit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else {
    return -1;
  }
}

static int decode_hex(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    return -1;
  }
}

static bool is_whitespace(char c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static void decode_whitespace(const char *text, size_t *offset) {
  while (is_whitespace(text[*offset])) {
    (*offset)++;
  }
}

static UtObject *decode_string(const char *text, size_t *offset) {
  size_t end = *offset;
  if (text[end] != '\"') {
    return NULL;
  }
  end++;

  UtObjectRef value = ut_mutable_string_new("");
  while (true) {
    if (text[end] == '\"') {
      *offset = end + 1;
      return ut_object_ref(value);
    }

    uint32_t code_point;
    if (text[end] == '\\') {
      switch (text[end + 1]) {
      case '\"':
        code_point = '"';
        break;
      case '\\':
        code_point = '\\';
        break;
      case '/':
        code_point = '/';
        break;
      case 'b':
        code_point = '\b';
        break;
      case 'f':
        code_point = '\f';
        break;
      case 'n':
        code_point = '\n';
        break;
      case 'r':
        code_point = '\r';
        break;
      case 't':
        code_point = '\t';
        break;
      case 'u':
        if (text[end + 2] == '\0' || text[end + 3] == '\0' ||
            text[end + 4] == '\0' || text[end + 5] == '\0') {
          return NULL;
        }
        int hex1 = decode_hex(text[end + 2]);
        int hex2 = decode_hex(text[end + 3]);
        int hex3 = decode_hex(text[end + 4]);
        int hex4 = decode_hex(text[end + 5]);
        if (hex1 < 0 || hex2 < 0 || hex3 < 0 || hex4 < 0) {
          // FIXME: Throw an error (invalid escape sequence)
          return NULL;
        }
        code_point = hex1 << 12 | hex2 << 8 | hex3 << 4 | hex4;
        end += 4;
        break;
      case '\0':
        return NULL;
      default:
        // FIXME: Throw an error (unknown escape sequence)
        return NULL;
      }
      end += 2;
    } else {
      if (!get_utf8_code_point(text, &end, &code_point)) {
        return NULL;
      }

      if (code_point <= 0x1f || code_point == 0x7f) {
        // FIXME: Throw an error (control characters not allowed)
        return NULL;
      }
    }

    ut_mutable_string_append_code_point(value, code_point);
  }
}

static UtObject *decode_number(const char *text, size_t *offset) {
  size_t end = *offset;

  int64_t sign = 1;
  if (text[end] == '-') {
    sign = -1;
    end++;
  }

  int64_t value = decode_digit(text[end]);
  if (value < 0) {
    // FIXME: Throw an error (invalid number)
    return NULL;
  }
  end++;
  if (value != 0) {
    int digit;
    while ((digit = decode_digit(text[end])) >= 0) {
      value = value * 10 + digit;
      end++;
    }
  }

  bool floating = false;
  double fraction = 0.0;
  if (text[end] == '.') {
    end++;

    floating = true;

    double numerator = decode_digit(text[end]);
    double denominator = 10;
    if (numerator < 0) {
      // FIXME: Throw an error
      return NULL;
    }
    end++;
    int digit;
    while ((digit = decode_digit(text[end])) >= 0) {
      numerator = numerator * 10 + digit;
      denominator *= 10;
      end++;
    }
    fraction = numerator / denominator;
  }

  int64_t exponent = 0;
  if (text[end] == 'e' || text[end] == 'E') {
    end++;

    int64_t exponent_sign = 1;
    if (text[end] == '+' || text[end] == '-') {
      exponent_sign = -1;
      end++;
    }

    exponent = decode_digit(text[end]);
    if (exponent < 0) {
      // FIXME: Throw an error
      return NULL;
    }
    end++;
    int digit;
    while ((digit = decode_digit(text[end])) >= 0) {
      exponent = exponent * 10 + digit;
      end++;
    }

    exponent *= exponent_sign;
  }
  if (exponent != 0) {
    floating = true;
  }

  *offset = end;
  if (floating) {
    double v = sign * (value + fraction);
    for (int64_t i = 0; i < exponent; i++) {
      v *= 10;
    }
    double divisor = 1;
    for (int64_t i = exponent; i < 0; i++) {
      divisor *= 10;
    }
    return ut_float64_new(v / divisor);
  } else {
    for (int64_t i = 0; i < exponent; i++) {
      value *= 10;
    }
    return ut_int64_new(sign * value);
  }
}

static UtObject *decode_object(const char *text, size_t *offset) {
  size_t end = *offset;
  if (text[end] != '{') {
    return NULL;
  }
  end++;

  decode_whitespace(text, &end);

  UtObjectRef object = ut_hash_table_new();
  if (text[end] == '}') {
    *offset = end + 1;
    return ut_object_ref(object);
  }

  while (true) {
    decode_whitespace(text, &end);
    UtObjectRef key = decode_string(text, &end);
    if (key == NULL) {
      // FIXME: Throw an error (invalid key in object)
      return NULL;
    }

    decode_whitespace(text, &end);
    if (text[end] != ':') {
      // FIXME: Throw an error (invalid object)
      return NULL;
    }
    end++;

    UtObjectRef value = decode_value(text, &end);
    if (value == NULL) {
      // FIXME: Throw an error (invalid value in object)
      return NULL;
    }

    ut_map_insert(object, key, value);

    if (text[end] == '}') {
      *offset = end + 1;
      return ut_object_ref(object);
    }

    if (text[end] != ',') {
      // FIXME: Throw an error (invalid character beween objects)
      return NULL;
    }
  }
}
static UtObject *decode_array(const char *text, size_t *offset) {
  size_t end = *offset;
  if (text[end] != '[') {
    return NULL;
  }
  end++;

  decode_whitespace(text, &end);

  UtObjectRef array = ut_object_array_new();
  if (text[end] == ']') {
    *offset = end + 1;
    return ut_object_ref(array);
  }

  while (true) {
    UtObject *value = decode_value(text, &end);
    if (value == NULL) {
      // FIXME: Throw an error (invalid value in array)
      return NULL;
    }

    ut_mutable_list_append_take(array, value);

    if (text[end] == ']') {
      *offset = end + 1;
      return ut_object_ref(array);
    }

    if (text[end] != ',') {
      // FIXME: Throw an error
      return NULL;
    }
  }
}

static UtObject *decode_value(const char *text, size_t *offset) {
  size_t end = *offset;
  decode_whitespace(text, &end);

  UtObject *value;
  switch (text[end]) {
  case '"':
    value = decode_string(text, &end);
    break;
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    value = decode_number(text, &end);
    break;
  case '{':
    value = decode_object(text, &end);
    break;
  case '[':
    value = decode_array(text, &end);
    break;
  case 't':
    if (text[end + 1] != 'r' || text[end + 2] != 'u' || text[end + 3] != 'e') {
      if (text[end + 1] != '\0' && text[end + 2] != '\0' &&
          text[end + 3] != '\0') {
        // FIXME: Throw an error (unknown keyword)
      }
      return NULL;
    }
    end += 4;
    value = ut_boolean_new(true);
    break;
  case 'f':
    if (text[end + 1] != 'a' || text[end + 2] != 'l' || text[end + 3] != 's' ||
        text[end + 4] != 'e') {
      if (text[end + 1] != '\0' && text[end + 2] != '\0' &&
          text[end + 3] != '\0' && text[end + 4] != '\0') {
        // FIXME: Throw an error (unknown keyword)
      }
      return NULL;
    }
    end += 5;
    value = ut_boolean_new(false);
    break;
  case 'n':
    if (text[end + 1] != 'u' || text[end + 2] != 'l' || text[end + 3] != 'l') {
      if (text[end + 1] != '\0' && text[end + 2] != '\0' &&
          text[end + 3] != '\0') {
        // FIXME: Throw an error (unknown keyword)
      }
      return NULL;
    }
    end += 4;
    value = ut_null_new();
    break;
  case '\0':
    return NULL;
  default:
    // FIXME: Throw an error (unknown value)
    return NULL;
  }
  if (value == NULL) {
    return NULL;
  }

  decode_whitespace(text, &end);

  *offset = end;
  return value;
}

char *ut_json_encode(UtObject *object) {
  UtObjectRef buffer = ut_mutable_string_new("");
  encode_value(buffer, object);
  return ut_string_take_text(buffer);
}

UtObject *ut_json_decode(const char *text) {
  size_t offset = 0;
  return decode_value(text, &offset);
}
