#include <stddef.h>
#include <stdint.h>

#include "ut-base64.h"
#include "ut-general-error.h"
#include "ut-list.h"
#include "ut-string.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

#define PADDING '='

static char value_to_char[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

#define INVALID 255

static uint8_t char_to_value[256] = {
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, 62,      INVALID, INVALID, INVALID, 63,
    52,      53,      54,      55,      56,      57,      58,      59,
    60,      61,      INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, 0,       1,       2,       3,       4,       5,       6,
    7,       8,       9,       10,      11,      12,      13,      14,
    15,      16,      17,      18,      19,      20,      21,      22,
    23,      24,      25,      INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, 26,      27,      28,      29,      30,      31,      32,
    33,      34,      35,      36,      37,      38,      39,      40,
    41,      42,      43,      44,      45,      46,      47,      48,
    49,      50,      51,      INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID};

static size_t decode_block(const char *text, size_t offset, UtObject *decoded) {
  char c1 = text[offset];
  uint8_t value1 = char_to_value[(uint8_t)c1];
  if (value1 == INVALID) {
    return 0;
  }
  char c2 = text[offset + 1];
  uint8_t value2 = char_to_value[(uint8_t)c2];
  if (value2 == INVALID) {
    return 0;
  }
  ut_uint8_array_append(decoded, value1 << 2 | value2 >> 4);
  size_t used = 2;
  char c3 = text[offset + 2];
  if (c3 != '\0') {
    uint8_t value3 = 0;
    used = 3;
    if (c3 != PADDING) {
      value3 = char_to_value[(uint8_t)c3];
      if (value3 == INVALID) {
        return 0;
      }
      ut_uint8_array_append(decoded, (value2 & 0x0f) << 4 | value3 >> 2);
    }
    char c4 = text[offset + 3];
    if (c3 == PADDING && c4 != PADDING) {
      return 0;
    }
    if (c4 != '\0') {
      used = 4;
      if (c4 != PADDING) {
        uint8_t value4 = char_to_value[(uint8_t)c4];
        if (value4 == INVALID) {
          return 0;
        }
        ut_uint8_array_append(decoded, (value3 & 0x03) << 6 | value4);
      }
    }
  }

  return used;
}

char *ut_base64_encode(UtObject *data) {
  UtObjectRef encoded = ut_string_new("");
  size_t length = ut_list_get_length(data), offset = 0;
  while (offset < length) {
    size_t available = length - offset;
    uint8_t byte1 = ut_uint8_list_get_element(data, offset);
    uint8_t byte2 =
        available > 1 ? ut_uint8_list_get_element(data, offset + 1) : 0;
    uint8_t byte3 =
        available > 2 ? ut_uint8_list_get_element(data, offset + 2) : 0;
    uint8_t value1 = byte1 >> 2;
    uint8_t value2 = ((byte1 & 0x03) << 4) | (byte2 >> 4);
    uint8_t value3 = ((byte2 & 0x0f) << 2) | (byte3 >> 6);
    uint8_t value4 = byte3 & 0x3f;
    ut_string_append_code_point(encoded, value_to_char[value1]);
    ut_string_append_code_point(encoded, value_to_char[value2]);
    ut_string_append_code_point(encoded, available > 1 ? value_to_char[value3]
                                                       : PADDING);
    ut_string_append_code_point(encoded, available > 2 ? value_to_char[value4]
                                                       : PADDING);
    offset += 3;
  }

  return ut_string_take_text(encoded);
}

UtObject *ut_base64_decode(const char *text) {
  UtObjectRef decoded = ut_uint8_array_new();
  size_t offset = 0;
  while (text[offset] != '\0') {
    size_t used = decode_block(text, offset, decoded);
    if (used == 0) {
      return ut_general_error_new("Invalid Base64");
    }
    offset += used;
  }

  return ut_object_ref(decoded);
}
