#include <assert.h>
#include <stdio.h>

#include "ut-boolean.h"
#include "ut-float64.h"
#include "ut-int64.h"
#include "ut-json-encoder.h"
#include "ut-list.h"
#include "ut-map-item.h"
#include "ut-map.h"
#include "ut-null.h"
#include "ut-string.h"
#include "ut-uint32-list.h"

typedef struct {
  UtObject object;
} UtJsonEncoder;

static bool encode_value(UtObject *buffer, UtObject *value);

static bool encode_string(UtObject *buffer, const char *value) {
  UtObjectRef input = ut_string_new_constant(value);
  UtObjectRef code_points = ut_string_get_code_points(input);
  size_t code_points_length = ut_list_get_length(code_points);
  ut_string_append(buffer, "\"");
  for (size_t i = 0; i < code_points_length; i++) {
    uint32_t code_point = ut_uint32_list_get_element(code_points, i);
    if (code_point <= 0x1f || code_point == 0x7f) {
      char escape_sequence[7];
      switch (code_point) {
      case '\b':
        ut_string_append(buffer, "\\b");
        break;
      case '\f':
        ut_string_append(buffer, "\\f");
        break;
      case '\n':
        ut_string_append(buffer, "\\n");
        break;
      case '\r':
        ut_string_append(buffer, "\\r");
        break;
      case '\t':
        ut_string_append(buffer, "\\t");
        break;
      default:
        snprintf(escape_sequence, 7, "\\u%04x", code_point);
        ut_string_append(buffer, escape_sequence);
        break;
      }
    } else if (code_point == '\"') {
      ut_string_append(buffer, "\\\"");
    } else if (code_point == '\\') {
      ut_string_append(buffer, "\\\\");
    } else {
      ut_string_append_code_point(buffer, code_point);
    }
  }

  ut_string_append(buffer, "\"");
  return true;
}

static bool encode_integer_number(UtObject *buffer, int64_t value) {
  char text[1024];
  snprintf(text, 1024, "%li", value);
  ut_string_append(buffer, text);
  return true;
}

static bool encode_float_number(UtObject *buffer, double value) {
  char text[1024];
  snprintf(text, 1024, "%e", value);
  ut_string_append(buffer, text);
  return true;
}

static bool encode_object(UtObject *buffer, UtObject *value) {
  ut_string_append(buffer, "{");
  UtObjectRef items = ut_map_get_items(value);
  size_t length = ut_list_get_length(items);
  for (size_t i = 0; i < length; i++) {
    UtObjectRef item = ut_list_get_element(items, i);
    if (i != 0) {
      ut_string_append(buffer, ",");
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

    ut_string_append(buffer, ":");

    UtObjectRef value = ut_map_item_get_value(item);
    result = encode_value(buffer, value);
    if (!result) {
      return false;
    }
  }
  ut_string_append(buffer, "}");
  return true;
}

static bool encode_array(UtObject *buffer, UtObject *value) {
  ut_string_append(buffer, "[");
  size_t length = ut_list_get_length(value);
  for (size_t i = 0; i < length; i++) {
    UtObjectRef child = ut_list_get_element(value, i);
    if (i != 0) {
      ut_string_append(buffer, ",");
    }
    bool result = encode_value(buffer, child);
    if (!result) {
      return false;
    }
  }
  ut_string_append(buffer, "]");
  return true;
}

static bool encode_boolean(UtObject *buffer, bool value) {
  ut_string_append(buffer, value ? "true" : "false");
  return true;
}

static bool encode_null(UtObject *buffer) {
  ut_string_append(buffer, "null");
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

static UtObjectInterface object_interface = {.type_name = "UtJsonEncoder",
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_json_encoder_new() {
  return ut_object_new(sizeof(UtJsonEncoder), &object_interface);
}

char *ut_json_encoder_encode(UtObject *object, UtObject *message) {
  assert(ut_object_is_json_encoder(object));

  UtObjectRef buffer = ut_string_new("");
  encode_value(buffer, message);
  return ut_string_take_text(buffer);
}

bool ut_object_is_json_encoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
