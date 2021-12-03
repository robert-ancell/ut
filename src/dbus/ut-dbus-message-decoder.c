#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "ut-boolean.h"
#include "ut-cstring.h"
#include "ut-dbus-array.h"
#include "ut-dbus-dict.h"
#include "ut-dbus-message-decoder.h"
#include "ut-dbus-message.h"
#include "ut-dbus-object-path.h"
#include "ut-dbus-signature.h"
#include "ut-dbus-struct.h"
#include "ut-dbus-variant.h"
#include "ut-file-descriptor.h"
#include "ut-float64.h"
#include "ut-input-stream.h"
#include "ut-int16.h"
#include "ut-int32.h"
#include "ut-int64.h"
#include "ut-list.h"
#include "ut-map.h"
#include "ut-object-list.h"
#include "ut-string.h"
#include "ut-uint16.h"
#include "ut-uint32.h"
#include "ut-uint64.h"
#include "ut-uint8-array-with-fds.h"
#include "ut-uint8-list.h"
#include "ut-uint8-subarray.h"
#include "ut-uint8.h"

typedef struct {
  UtObject object;
  UtObject *input_stream;
  UtObject *fds;
  UtObject *messages;
  UtInputStreamCallback callback;
  void *user_data;
} UtDBusMessageDecoder;

static UtObject *read_value(UtObject *data, UtObject *fds, size_t *offset,
                            const char *signature);

static UtObject *read_values(UtObject *data, UtObject *fds, size_t *offset,
                             const char *signature);

static size_t get_remaining(UtObject *data, size_t *offset) {
  return ut_list_get_length(data) - *offset;
}

static UtObject *read_byte(UtObject *data, size_t *offset) {
  if (get_remaining(data, offset) < 1) {
    return NULL;
  }
  uint8_t value = ut_uint8_list_get_element(data, *offset);
  (*offset)++;
  return ut_uint8_new(value);
}

static size_t get_alignment(const char *signature) {
  switch (signature[0]) {
  case 'y': // byte
  case 'g': // signature
  case 'v': // variant
    return 1;
  case 'n': // int16
  case 'q': // uint16
    return 2;
  case 'b': // boolean
  case 'i': // int32
  case 'u': // uint32
  case 's': // string
  case 'o': // object path
  case 'a': // array
  case 'h': // unix fd
    return 4;
  case 'x': // int64
  case 't': // uint64
  case 'd': // double
  case '(': // struct
  case '{': // dict entry
    return 8;
  default:
    return 1;
  }
}

static bool read_align_padding(UtObject *data, size_t *offset,
                               size_t alignment) {
  size_t extra = *offset % alignment;
  if (extra != 0) {
    size_t count = alignment - extra;
    if (get_remaining(data, offset) < count) {
      return false;
    }
    *offset += count;
  }

  return true;
}

static UtObject *read_int16(UtObject *data, size_t *offset) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 2) || get_remaining(data, &o) < 2) {
    return NULL;
  }
  int16_t value = ut_uint8_list_get_int16_le(data, o);
  *offset = o + 2;
  return ut_int16_new(value);
}

static UtObject *read_uint16(UtObject *data, size_t *offset) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 2) || get_remaining(data, &o) < 2) {
    return NULL;
  }
  uint16_t value = ut_uint8_list_get_uint16_le(data, o);
  *offset = o + 2;
  return ut_uint16_new(value);
}

static UtObject *read_int32(UtObject *data, size_t *offset) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 4) || get_remaining(data, &o) < 4) {
    return NULL;
  }
  int32_t value = ut_uint8_list_get_int32_le(data, o);
  *offset = o + 4;
  return ut_int32_new(value);
}

static UtObject *read_uint32(UtObject *data, size_t *offset) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 4) || get_remaining(data, &o) < 4) {
    return NULL;
  }
  uint32_t value = ut_uint8_list_get_uint32_le(data, o);
  *offset = o + 4;
  return ut_uint32_new(value);
}

static UtObject *read_boolean(UtObject *data, size_t *offset) {
  UtObjectRef value = read_uint32(data, offset);
  if (value == NULL) {
    return NULL;
  }

  return ut_boolean_new(ut_uint32_get_value(value) != 0);
}

static UtObject *read_int64(UtObject *data, size_t *offset) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 8) || get_remaining(data, &o) < 8) {
    return NULL;
  }
  int64_t value = ut_uint8_list_get_int64_le(data, o);
  *offset = o + 8;
  return ut_int64_new(value);
}

static UtObject *read_uint64(UtObject *data, size_t *offset) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 8) || get_remaining(data, &o) < 8) {
    return NULL;
  }
  uint64_t value = ut_uint8_list_get_uint64_le(data, o);
  *offset = o + 8;
  return ut_uint64_new(value);
}

static UtObject *read_double(UtObject *data, size_t *offset) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 8) || get_remaining(data, &o) < 8) {
    return NULL;
  }
  double value = ut_uint8_list_get_float64_le(data, o);
  *offset = o + 8;
  return ut_float64_new(value);
}

static UtObject *read_string(UtObject *data, size_t *offset) {
  size_t o = *offset;
  UtObjectRef value_length_object = read_uint32(data, &o);
  if (value_length_object == NULL) {
    return NULL;
  }
  size_t value_length = ut_uint32_get_value(value_length_object);
  if (get_remaining(data, &o) < value_length + 1) {
    return NULL;
  }
  UtObjectRef utf8 = ut_uint8_list_new();
  for (size_t i = 0; i < value_length; i++) {
    ut_uint8_list_append(utf8, ut_uint8_list_get_element(data, o + i));
  }
  *offset = o + value_length + 1;
  return ut_string_new_from_utf8(utf8);
}

static UtObject *read_object_path(UtObject *data, size_t *offset) {
  UtObjectRef value = read_string(data, offset);
  if (value == NULL) {
    return NULL;
  }
  return ut_dbus_object_path_new(ut_string_get_text(value));
}

static UtObject *read_signature(UtObject *data, size_t *offset) {
  size_t remaining = get_remaining(data, offset);
  if (remaining < 1) {
    return NULL;
  }
  size_t signature_length = ut_uint8_list_get_element(data, *offset);
  if (remaining < 2 + signature_length) {
    return NULL;
  }
  ut_cstring_ref value = malloc(sizeof(char) * (signature_length + 1));
  for (size_t i = 0; i < signature_length + 1; i++) {
    value[i] = ut_uint8_list_get_element(data, *offset + 1 + i);
  }
  *offset += 2 + signature_length;
  return ut_dbus_signature_new(value);
}

static UtObject *read_struct(UtObject *data, UtObject *fds, size_t *offset,
                             const char *signatures) {
  size_t o = *offset;
  if (!read_align_padding(data, &o, 8)) {
    return NULL;
  }
  UtObjectRef values = read_values(data, fds, &o, signatures);
  if (values == NULL) {
    return NULL;
  }
  *offset = o;
  return ut_dbus_struct_new_from_list(values);
}

static UtObject *read_array(UtObject *data, UtObject *fds, size_t *offset,
                            const char *signature) {
  size_t o = *offset;
  UtObjectRef length_object = read_uint32(data, &o);
  if (length_object == NULL) {
    return NULL;
  }
  if (!read_align_padding(data, &o, get_alignment(signature))) {
    return NULL;
  }
  size_t length = ut_uint32_get_value(length_object);
  if (get_remaining(data, &o) < length) {
    return NULL;
  }
  size_t end_offset = o + length;
  UtObjectRef result = ut_dbus_array_new(signature);
  while (o < end_offset) {
    UtObjectRef value = read_value(data, fds, &o, signature);
    ut_list_append(result, value);
  }
  assert(o <= end_offset);
  *offset = end_offset;
  return ut_object_ref(result);
}

static UtObject *read_variant(UtObject *data, UtObject *fds, size_t *offset) {
  UtObjectRef signature = read_signature(data, offset);
  if (signature == NULL) {
    return NULL;
  }
  UtObjectRef value =
      read_value(data, fds, offset, ut_dbus_signature_get_value(signature));
  if (value == NULL) {
    return NULL;
  }
  return ut_dbus_variant_new(value);
}

static UtObject *read_unix_fd(UtObject *data, UtObject *fds, size_t *offset) {
  UtObjectRef index_object = read_uint32(data, offset);
  if (index_object == NULL) {
    return NULL;
  }
  size_t index = ut_uint32_get_value(index_object);
  assert(fds != NULL);
  assert(index < ut_list_get_length(fds));
  return ut_list_get_element(fds, index);
}

static UtObject *read_dict(UtObject *data, UtObject *fds, size_t *offset,
                           const char *signature) {
  UtObjectRef signature_object = ut_dbus_signature_new(signature);
  UtObjectRef signatures = ut_dbus_signature_split(signature_object);
  size_t signatures_length = ut_list_get_length(signatures);
  assert(signatures_length == 2);
  UtObjectRef key_signature = ut_list_get_element(signatures, 0);
  UtObjectRef value_signature = ut_list_get_element(signatures, 1);
  // FIXME: check key_signature is a basic type

  size_t o = *offset;
  UtObjectRef length_object = read_uint32(data, &o);
  if (length_object == NULL) {
    return NULL;
  }
  if (!read_align_padding(data, &o, 8)) {
    return NULL;
  }
  size_t length = ut_uint32_get_value(length_object);
  if (get_remaining(data, &o) < length) {
    return NULL;
  }
  size_t end_offset = o + length;
  UtObjectRef result =
      ut_dbus_dict_new(ut_dbus_signature_get_value(key_signature),
                       ut_dbus_signature_get_value(value_signature));
  while (o < end_offset) {
    UtObjectRef entry = read_struct(data, fds, &o, signature);
    ut_map_insert(result, ut_dbus_struct_get_value(entry, 0),
                  ut_dbus_struct_get_value(entry, 1));
  }
  assert(o <= end_offset);
  *offset = end_offset;
  return ut_object_ref(result);
}

static UtObject *read_value(UtObject *data, UtObject *fds, size_t *offset,
                            const char *signature) {
  if (strcmp(signature, "y") == 0) {
    return read_byte(data, offset);
  } else if (strcmp(signature, "b") == 0) {
    return read_boolean(data, offset);
  } else if (strcmp(signature, "n") == 0) {
    return read_int16(data, offset);
  } else if (strcmp(signature, "q") == 0) {
    return read_uint16(data, offset);
  } else if (strcmp(signature, "i") == 0) {
    return read_int32(data, offset);
  } else if (strcmp(signature, "u") == 0) {
    return read_uint32(data, offset);
  } else if (strcmp(signature, "x") == 0) {
    return read_int64(data, offset);
  } else if (strcmp(signature, "t") == 0) {
    return read_uint64(data, offset);
  } else if (strcmp(signature, "d") == 0) {
    return read_double(data, offset);
  } else if (strcmp(signature, "s") == 0) {
    return read_string(data, offset);
  } else if (strcmp(signature, "o") == 0) {
    return read_object_path(data, offset);
  } else if (strcmp(signature, "g") == 0) {
    return read_signature(data, offset);
  } else if (strncmp(signature, "(", 1) == 0) {
    assert(ut_cstring_ends_with(signature, ")"));
    ut_cstring_ref struct_signature =
        ut_cstring_substring(signature, 1, strlen(signature) - 1);
    return read_struct(data, fds, offset, struct_signature);
  } else if (strncmp(signature, "a{", 2) == 0) {
    assert(ut_cstring_ends_with(signature, "}"));
    ut_cstring_ref element_signature =
        ut_cstring_substring(signature, 2, strlen(signature) - 1);
    return read_dict(data, fds, offset, element_signature);
  } else if (strncmp(signature, "a", 1) == 0) {
    return read_array(data, fds, offset, signature + 1);
  } else if (strcmp(signature, "v") == 0) {
    return read_variant(data, fds, offset);
  } else if (strcmp(signature, "h") == 0) {
    return read_unix_fd(data, fds, offset);
  } else {
    assert(false);
  }
}

static UtObject *read_values(UtObject *data, UtObject *fds, size_t *offset,
                             const char *signature) {
  UtObjectRef signature_object = ut_dbus_signature_new(signature);
  UtObjectRef signatures = ut_dbus_signature_split(signature_object);
  size_t signatures_length = ut_list_get_length(signatures);

  UtObjectRef values = ut_list_new();
  size_t o = *offset;
  for (size_t i = 0; i < signatures_length; i++) {
    UtObjectRef value_signature = ut_list_get_element(signatures, i);
    UtObjectRef value =
        read_value(data, fds, &o, ut_dbus_signature_get_value(value_signature));
    if (value == NULL) {
      return NULL;
    }
    ut_list_append(values, value);
  }

  *offset = o;
  return ut_object_ref(values);
}

static UtObject *read_message(UtDBusMessageDecoder *self, UtObject *data,
                              size_t *offset) {
  size_t o = *offset;
  UtObjectRef header = read_values(data, NULL, &o, "yyyyuua(yv)");
  if (header == NULL) {
    return NULL;
  }

  uint8_t endianess = ut_uint8_get_value(ut_object_list_get_element(header, 0));
  uint8_t type = ut_uint8_get_value(ut_object_list_get_element(header, 1));
  uint8_t flags = ut_uint8_get_value(ut_object_list_get_element(header, 2));
  uint8_t protocol_version =
      ut_uint8_get_value(ut_object_list_get_element(header, 3));
  uint32_t body_length =
      ut_uint32_get_value(ut_object_list_get_element(header, 4));
  uint32_t serial = ut_uint32_get_value(ut_object_list_get_element(header, 5));
  UtObject *header_fields = ut_object_list_get_element(header, 6);

  assert(endianess == 'l');
  assert(protocol_version == 1);

  size_t header_fields_length = ut_list_get_length(header_fields);
  const char *path = NULL, *interface = NULL, *member = NULL,
             *error_name = NULL, *body_signature = NULL, *destination = NULL,
             *sender = NULL;
  bool has_reply_serial = false;
  uint32_t reply_serial = 0;
  size_t fds_length = 0;
  for (size_t i = 0; i < header_fields_length; i++) {
    UtObject *header_field = ut_object_list_get_element(header_fields, i);
    uint8_t code =
        ut_uint8_get_value(ut_object_list_get_element(header_field, 0));
    UtObject *value =
        ut_dbus_variant_get_value(ut_object_list_get_element(header_field, 1));

    switch (code) {
    case 0:
      assert(false);
      break;
    case 1:
      assert(ut_object_is_dbus_object_path(value));
      assert(path == NULL);
      path = ut_dbus_object_path_get_value(value);
      break;
    case 2:
      assert(ut_object_implements_string(value));
      assert(interface == NULL);
      interface = ut_string_get_text(value);
      break;
    case 3:
      assert(ut_object_implements_string(value));
      assert(member == NULL);
      member = ut_string_get_text(value);
      break;
    case 4:
      assert(ut_object_implements_string(value));
      assert(error_name == NULL);
      error_name = ut_string_get_text(value);
      break;
    case 5:
      assert(ut_object_is_uint32(value));
      assert(!has_reply_serial);
      has_reply_serial = true;
      reply_serial = ut_uint32_get_value(value);
      break;
    case 6:
      assert(ut_object_implements_string(value));
      assert(destination == NULL);
      destination = ut_string_get_text(value);
      break;
    case 7:
      assert(ut_object_implements_string(value));
      assert(sender == NULL);
      sender = ut_string_get_text(value);
      break;
    case 8:
      assert(ut_object_is_dbus_signature(value));
      assert(body_signature == NULL);
      body_signature = ut_dbus_signature_get_value(value);
      break;
    case 9:
      assert(ut_object_is_uint32(value));
      assert(fds_length == 0);
      fds_length = ut_uint32_get_value(value);
      assert(ut_list_get_length(self->fds) >= fds_length);
      break;
    }
  }

  if (!read_align_padding(data, &o, 8)) {
    return NULL;
  }

  if (get_remaining(data, &o) < body_length) {
    return NULL;
  }

  UtObjectRef fds = ut_list_get_sublist(self->fds, 0, fds_length);

  size_t body_end = o + body_length;
  UtObjectRef args = NULL;
  if (body_signature != NULL) {
    args = read_values(data, fds, &o, body_signature);
    assert(args != NULL);
    assert(o <= body_end);
  } else {
    assert(body_length == 0);
  }
  *offset = body_end;

  ut_list_remove(self->fds, 0, fds_length);

  UtObject *message = ut_dbus_message_new(type);
  ut_dbus_message_set_flags(message, flags);
  ut_dbus_message_set_serial(message, serial);
  ut_dbus_message_set_path(message, path);
  ut_dbus_message_set_interface(message, interface);
  ut_dbus_message_set_member(message, member);
  ut_dbus_message_set_error_name(message, error_name);
  if (has_reply_serial) {
    ut_dbus_message_set_reply_serial(message, reply_serial);
  }
  ut_dbus_message_set_destination(message, destination);
  ut_dbus_message_set_sender(message, sender);
  ut_dbus_message_set_args(message, args);

  return message;
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtDBusMessageDecoder *self = user_data;

  if (ut_object_is_uint8_array_with_fds(data)) {
    ut_list_append_list(self->fds, ut_uint8_array_with_fds_get_fds(data));
    data = ut_uint8_array_with_fds_get_data(data);
  }

  size_t offset = 0;
  size_t data_length = ut_list_get_length(data);
  while (true) {
    UtObjectRef d = ut_uint8_subarray_new(data, offset, data_length - offset);
    size_t o = 0;
    UtObjectRef message = read_message(self, d, &o);
    if (message == NULL) {
      break;
    }

    ut_list_append(self->messages, message);

    offset += o;
  }

  if (ut_list_get_length(self->messages) > 0) {
    size_t n_used = self->callback(self->user_data, self->messages, complete);
    ut_list_remove(self->messages, 0, n_used);
  }

  return offset;
}

static void ut_dbus_message_decoder_init(UtObject *object) {
  UtDBusMessageDecoder *self = (UtDBusMessageDecoder *)object;
  self->input_stream = NULL;
  self->fds = ut_object_list_new();
  self->messages = ut_object_list_new();
  self->callback = NULL;
  self->user_data = NULL;
}

static void ut_dbus_message_decoder_cleanup(UtObject *object) {
  UtDBusMessageDecoder *self = (UtDBusMessageDecoder *)object;
  ut_object_unref(self->input_stream);
  ut_object_unref(self->messages);
}

static void
ut_dbus_message_decoder_input_stream_read(UtObject *object,
                                          UtInputStreamCallback callback,
                                          void *user_data, UtObject *cancel) {
  UtDBusMessageDecoder *self = (UtDBusMessageDecoder *)object;
  assert(callback != NULL);

  assert(self->callback == NULL);
  self->callback = callback;
  self->user_data = user_data;
  ut_input_stream_read(self->input_stream, read_cb, self, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_dbus_message_decoder_input_stream_read};

static UtObjectInterface object_interface = {
    .type_name = "UtDBusMessageDecoder",
    .init = ut_dbus_message_decoder_init,
    .cleanup = ut_dbus_message_decoder_cleanup,
    .interfaces = {{
                       &ut_input_stream_id,
                       &input_stream_interface,
                   },
                   {NULL, NULL}}};

UtObject *ut_dbus_message_decoder_new(UtObject *input_stream) {
  assert(input_stream != NULL);

  UtObject *object =
      ut_object_new(sizeof(UtDBusMessageDecoder), &object_interface);
  UtDBusMessageDecoder *self = (UtDBusMessageDecoder *)object;
  self->input_stream = ut_object_ref(input_stream);
  return object;
}

bool ut_object_is_dbus_message_decoder(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
