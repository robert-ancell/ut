#include <assert.h>
#include <stdio.h>

#include "ut-boolean.h"
#include "ut-float64.h"
#include "ut-hash-table.h"
#include "ut-input-stream.h"
#include "ut-int64.h"
#include "ut-json-decoder.h"
#include "ut-list.h"
#include "ut-map.h"
#include "ut-null.h"
#include "ut-object-array.h"
#include "ut-string.h"
#include "ut-uint32-list.h"
#include "ut-utf8-decoder.h"

typedef struct {
  UtObject object;
  UtObject *utf8_decoder;
  UtInputStreamCallback callback;
  void *user_data;
} UtJsonDecoder;

static UtObject *decode_value(UtObject *text, size_t *offset,
                              bool complete_data);

static int decode_digit(uint32_t c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else {
    return -1;
  }
}

static int decode_hex(uint32_t c) {
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

static bool is_whitespace(uint32_t c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static void decode_whitespace(UtObject *text, size_t *offset) {
  size_t text_length = ut_list_get_length(text);
  while (*offset < text_length &&
         is_whitespace(ut_uint32_list_get_element(text, *offset))) {
    (*offset)++;
  }
}

static UtObject *decode_string(UtObject *text, size_t *offset) {
  size_t end = *offset;
  uint32_t c = ut_uint32_list_get_element(text, end);
  if (c != '\"') {
    return NULL;
  }
  end++;

  UtObjectRef value = ut_string_new("");
  size_t text_length = ut_list_get_length(text);
  while (end < text_length) {
    c = ut_uint32_list_get_element(text, end);
    if (c == '\"') {
      *offset = end + 1;
      return ut_object_ref(value);
    }

    if (c == '\\') {
      if (end + 1 >= text_length) {
        return NULL;
      }
      c = ut_uint32_list_get_element(text, end + 1);
      switch (c) {
      case '\"':
        c = '"';
        break;
      case '\\':
        c = '\\';
        break;
      case '/':
        c = '/';
        break;
      case 'b':
        c = '\b';
        break;
      case 'f':
        c = '\f';
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      case 'u':
        if (end + 5 >= text_length) {
          return NULL;
        }
        int hex1 = decode_hex(ut_uint32_list_get_element(text, end + 2));
        int hex2 = decode_hex(ut_uint32_list_get_element(text, end + 3));
        int hex3 = decode_hex(ut_uint32_list_get_element(text, end + 4));
        int hex4 = decode_hex(ut_uint32_list_get_element(text, end + 5));
        if (hex1 < 0 || hex2 < 0 || hex3 < 0 || hex4 < 0) {
          // FIXME: Throw an error (invalid escape sequence)
          return NULL;
        }
        c = hex1 << 12 | hex2 << 8 | hex3 << 4 | hex4;
        end += 4;
        break;
      default:
        // FIXME: Throw an error (unknown escape sequence)
        return NULL;
      }
      end += 2;
    } else {
      if (c <= 0x1f || c == 0x7f) {
        // FIXME: Throw an error (control characters not allowed)
        return NULL;
      }
      end++;
    }

    ut_string_append_code_point(value, c);
  }

  return NULL;
}

static UtObject *decode_number(UtObject *text, size_t *offset,
                               bool complete_data) {
  size_t text_length = ut_list_get_length(text);
  size_t end = *offset;

  int64_t sign = 1;
  if (ut_uint32_list_get_element(text, end) == '-') {
    sign = -1;
    end++;
  }

  int64_t value = decode_digit(ut_uint32_list_get_element(text, end));
  if (value < 0) {
    // FIXME: Throw an error (invalid number)
    return NULL;
  }
  end++;
  if (value != 0) {
    int digit;
    while ((digit = decode_digit(ut_uint32_list_get_element(text, end))) >= 0) {
      value = value * 10 + digit;
      end++;
    }
  }

  bool floating = false;
  double fraction = 0.0;
  if (ut_uint32_list_get_element(text, end) == '.') {
    end++;

    floating = true;

    double numerator = decode_digit(ut_uint32_list_get_element(text, end));
    double denominator = 10;
    if (numerator < 0) {
      // FIXME: Throw an error
      return NULL;
    }
    end++;
    int digit;
    while ((digit = decode_digit(ut_uint32_list_get_element(text, end))) >= 0) {
      numerator = numerator * 10 + digit;
      denominator *= 10;
      end++;
    }
    fraction = numerator / denominator;
  }

  int64_t exponent = 0;
  uint32_t c = ut_uint32_list_get_element(text, end);
  if (c == 'e' || c == 'E') {
    end++;

    int64_t exponent_sign = 1;
    uint32_t c = ut_uint32_list_get_element(text, end);
    if (c == '+' || c == '-') {
      exponent_sign = -1;
      end++;
    }

    exponent = decode_digit(ut_uint32_list_get_element(text, end));
    if (exponent < 0) {
      // FIXME: Throw an error
      return NULL;
    }
    end++;
    int digit;
    while (end < text_length &&
           (digit = decode_digit(ut_uint32_list_get_element(text, end))) >= 0) {
      exponent = exponent * 10 + digit;
      end++;
    }

    exponent *= exponent_sign;
  }
  if (exponent != 0) {
    floating = true;
  }

  // Don't know if we have all the number.
  if (!complete_data && end >= text_length) {
    return NULL;
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

static UtObject *decode_object(UtObject *text, size_t *offset) {
  size_t text_length = ut_list_get_length(text);

  size_t end = *offset;
  if (end >= text_length) {
    return NULL;
  }
  if (ut_uint32_list_get_element(text, end) != '{') {
    return NULL;
  }
  end++;

  decode_whitespace(text, &end);

  UtObjectRef object = ut_hash_table_new();
  if (end >= text_length) {
    return NULL;
  }
  if (ut_uint32_list_get_element(text, end) == '}') {
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
    if (end >= text_length) {
      return NULL;
    }
    if (ut_uint32_list_get_element(text, end) != ':') {
      // FIXME: Throw an error (invalid object)
      return NULL;
    }
    end++;

    UtObjectRef value = decode_value(text, &end, false);
    if (value == NULL) {
      // FIXME: Throw an error (invalid value in object)
      return NULL;
    }

    ut_map_insert(object, key, value);

    if (end >= text_length) {
      return NULL;
    }
    uint32_t c = ut_uint32_list_get_element(text, end);
    if (c == '}') {
      *offset = end + 1;
      return ut_object_ref(object);
    }

    if (c != ',') {
      // FIXME: Throw an error (invalid character beween objects)
      return NULL;
    }
    end++;
  }
}

static UtObject *decode_array(UtObject *text, size_t *offset) {
  size_t text_length = ut_list_get_length(text);

  size_t end = *offset;
  if (end >= text_length) {
    return NULL;
  }
  if (ut_uint32_list_get_element(text, end) != '[') {
    return NULL;
  }
  end++;

  decode_whitespace(text, &end);

  UtObjectRef array = ut_object_array_new();
  if (end >= text_length) {
    return NULL;
  }
  if (ut_uint32_list_get_element(text, end) == ']') {
    *offset = end + 1;
    return ut_object_ref(array);
  }

  while (true) {
    UtObjectRef value = decode_value(text, &end, false);
    if (value == NULL) {
      // FIXME: Throw an error (invalid value in array)
      return NULL;
    }

    ut_list_append(array, value);

    if (end >= text_length) {
      return NULL;
    }
    uint32_t c = ut_uint32_list_get_element(text, end);
    if (c == ']') {
      *offset = end + 1;
      return ut_object_ref(array);
    }

    if (c != ',') {
      // FIXME: Throw an error
      return NULL;
    }
    end++;
  }
}

static UtObject *decode_value(UtObject *text, size_t *offset,
                              bool complete_data) {
  size_t text_length = ut_list_get_length(text);

  size_t end = *offset;
  decode_whitespace(text, &end);

  if (end >= text_length) {
    return NULL;
  }

  UtObject *value;
  switch (ut_uint32_list_get_element(text, end)) {
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
    value = decode_number(text, &end, complete_data);
    break;
  case '{':
    value = decode_object(text, &end);
    break;
  case '[':
    value = decode_array(text, &end);
    break;
  case 't':
    if (end + 3 >= text_length) {
      return NULL;
    }
    if (ut_uint32_list_get_element(text, end + 1) != 'r' ||
        ut_uint32_list_get_element(text, end + 2) != 'u' ||
        ut_uint32_list_get_element(text, end + 3) != 'e') {
      // FIXME: Throw an error (unknown keyword)
      return NULL;
    }

    end += 4;
    value = ut_boolean_new(true);
    break;
  case 'f':
    if (end + 4 >= text_length) {
      return NULL;
    }
    if (ut_uint32_list_get_element(text, end + 1) != 'a' ||
        ut_uint32_list_get_element(text, end + 2) != 'l' ||
        ut_uint32_list_get_element(text, end + 3) != 's' ||
        ut_uint32_list_get_element(text, end + 4) != 'e') {
      // FIXME: Throw an error (unknown keyword)
      return NULL;
    }
    end += 5;
    value = ut_boolean_new(false);
    break;
  case 'n':
    if (end + 3 >= text_length) {
      return NULL;
    }
    if (ut_uint32_list_get_element(text, end + 1) != 'u' ||
        ut_uint32_list_get_element(text, end + 2) != 'l' ||
        ut_uint32_list_get_element(text, end + 3) != 'l') {
      // FIXME: Throw an error (unknown keyword)
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

static void ut_json_decoder_init(UtObject *object) {
  UtJsonDecoder *self = (UtJsonDecoder *)object;
  self->utf8_decoder = NULL;
  self->callback = NULL;
  self->user_data = NULL;
}

static void ut_json_decoder_cleanup(UtObject *object) {
  UtJsonDecoder *self = (UtJsonDecoder *)object;
  ut_object_unref(self->utf8_decoder);
}

static size_t read_cb(void *user_data, UtObject *data) {
  // UtJsonDecoder *self = user_data;

  size_t offset = 0;
  UtObjectRef value = decode_value(data, &offset, true);

  return offset;
}

static void ut_json_decoder_input_stream_read(UtObject *object,
                                              UtInputStreamCallback callback,
                                              void *user_data,
                                              UtObject *cancel) {
  UtJsonDecoder *self = (UtJsonDecoder *)object;
  assert(callback != NULL);

  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  ut_input_stream_read(self->utf8_decoder, read_cb, self, cancel);
}

static void
ut_json_decoder_input_stream_read_all(UtObject *object,
                                      UtInputStreamCallback callback,
                                      void *user_data, UtObject *cancel) {
  UtJsonDecoder *self = (UtJsonDecoder *)object;
  assert(callback != NULL);

  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  ut_input_stream_read_all(self->utf8_decoder, read_cb, self, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_json_decoder_input_stream_read,
    .read_all = ut_json_decoder_input_stream_read_all};

static UtObjectInterface object_interface = {
    .type_name = "UtJsonDecoder",
    .init = ut_json_decoder_init,
    .cleanup = ut_json_decoder_cleanup,
    .interfaces = {{
                       &ut_input_stream_id,
                       &input_stream_interface,
                   },
                   {NULL, NULL}}};

UtObject *ut_json_decoder_new(UtObject *input_stream) {
  assert(input_stream != NULL);

  UtObject *object = ut_object_new(sizeof(UtJsonDecoder), &object_interface);
  UtJsonDecoder *self = (UtJsonDecoder *)object;
  self->utf8_decoder = ut_utf8_decoder_new(input_stream);
  return object;
}

UtObject *ut_json_decode(const char *text) {
  UtObjectRef string = ut_string_new_constant(text);
  UtObjectRef code_points = ut_string_get_code_points(string);
  size_t offset = 0;
  return decode_value(code_points, &offset, true);
}

bool ut_object_is_json_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
