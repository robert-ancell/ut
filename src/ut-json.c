#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ut-boolean.h"
#include "ut-hash-map.h"
#include "ut-int64.h"
#include "ut-json.h"
#include "ut-map.h"
#include "ut-mutable-list.h"
#include "ut-mutable-string.h"
#include "ut-null.h"
#include "ut-object-array.h"

static UtObject *decode_value(const char *text, size_t *offset);

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

  UtObject *value = ut_mutable_string_new("");
  while (true) {
    if (text[end] == '\"') {
      *offset = end + 1;
      return value;
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
        ut_object_unref(value);
        return NULL;
      }
      end += 2;
    } else {
      uint8_t byte1 = text[end];
      if (byte1 <= 0x1f || byte1 == 0x7f) {
        // FIXME: Throw an error (control characters not allowed)
        ut_object_unref(value);
        return NULL;
      } else if ((byte1 & 0x80) == 0) {
        code_point = byte1;
        end++;
      } else if ((byte1 & 0xe0) == 0xc0) {
        if (text[end + 1] == '\0') {
          return NULL;
        }
        uint8_t byte2 = text[end + 1];
        if ((byte1 & 0xc0) != 0x80) {
          // FIXME: Throw an error (invalid utf-8)
          return NULL;
        }
        code_point = (byte1 & 0x1f) << 6 | (byte2 & 0x3f);
        end += 2;
      } else if ((byte1 & 0xf0) == 0xe0) {
        if (text[end + 1] == '\0' || text[end + 2] == '\0') {
          return NULL;
        }
        uint8_t byte2 = text[end + 1];
        uint8_t byte3 = text[end + 2];
        code_point =
            (byte1 & 0x0f) << 12 | (byte2 & 0x3f) << 6 | (byte3 & 0x3f);
        end += 3;
      } else if ((byte1 & 0xf8) == 0xf0) {
        if (text[end + 1] == '\0' || text[end + 2] == '\0' ||
            text[end + 3] == '\0') {
          return NULL;
        }
        uint8_t byte2 = text[end + 1];
        uint8_t byte3 = text[end + 2];
        uint8_t byte4 = text[end + 3];
        code_point = (byte1 & 0x07) << 18 | (byte2 & 0x3f) << 12 |
                     (byte3 & 0x3f) << 6 | (byte4 & 0x3f);
        end += 4;
      } else {
        // FIXME: Throw an error (invalid utf-8)
        ut_object_unref(value);
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

  if (text[end] == '.') {
    end++;
    int digit = decode_digit(text[end]);
    if (digit < 0) {
      // FIXME: Throw an error
      return NULL;
    }
    end++;
    while ((digit = decode_digit(text[end])) >= 0) {
      // FIXME
      end++;
    }
  }

  if (text[end] == 'e' || text[end] == 'E') {
    end++;
    if (text[end] == '+' || text[end] == '-') {
      end++;
    }

    int digit = decode_digit(text[end]);
    if (digit < 0) {
      // FIXME: Throw an error
      return NULL;
    }
    end++;
    while ((digit = decode_digit(text[end])) >= 0) {
      // FIXME
      end++;
    }
  }

  *offset = end;
  return ut_int64_new(sign * value);
}

static UtObject *decode_object(const char *text, size_t *offset) {
  size_t end = *offset;
  if (text[end] != '{') {
    return NULL;
  }
  end++;

  decode_whitespace(text, &end);

  UtObject *object = ut_hash_map_new();
  if (text[end] == '}') {
    *offset = end + 1;
    return object;
  }

  while (true) {
    decode_whitespace(text, &end);
    UtObject *key = decode_string(text, &end);
    if (key == NULL) {
      // FIXME: Throw an error (invalid key in object)
      ut_object_unref(object);
      return NULL;
    }

    decode_whitespace(text, &end);
    if (text[end] != ':') {
      ut_object_unref(key);
      ut_object_unref(object);
      // FIXME: Throw an error (invalid object)
      return NULL;
    }
    end++;

    UtObject *value = decode_value(text, &end);
    if (value == NULL) {
      ut_object_unref(key);
      ut_object_unref(object);
      // FIXME: Throw an error (invalid value in object)
      return NULL;
    }

    ut_map_insert_take(object, key, value);

    if (text[end] == '}') {
      *offset = end + 1;
      return object;
    }

    if (text[end] != ',') {
      ut_object_unref(object);
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

  UtObject *array = ut_object_array_new();
  if (text[end] == ']') {
    *offset = end + 1;
    return array;
  }

  while (true) {
    UtObject *value = decode_value(text, &end);
    if (value == NULL) {
      ut_object_unref(array);
      // FIXME: Throw an error (invalid value in array)
      return NULL;
    }

    ut_mutable_list_append_take(array, value);

    if (text[end] == ']') {
      *offset = end + 1;
      return array;
    }

    if (text[end] != ',') {
      ut_object_unref(array);
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

char *ut_json_encode(UtObject *object) { return strdup(""); }

UtObject *ut_json_decode(const char *text) {
  size_t offset = 0;
  return decode_value(text, &offset);
}
