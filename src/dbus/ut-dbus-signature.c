#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-cstring.h"
#include "ut-dbus-signature.h"
#include "ut-general-error.h"
#include "ut-list.h"

typedef struct {
  UtObject object;
  char *value;
} UtDBusSignature;

static void ut_dbus_signature_init(UtObject *object) {
  UtDBusSignature *self = (UtDBusSignature *)object;
  self->value = NULL;
}

static char *ut_dbus_signature_to_string(UtObject *object) {
  UtDBusSignature *self = (UtDBusSignature *)object;
  return ut_cstring_new_printf("<UtDBusSignature>(\"%s\")", self->value);
}

static void ut_dbus_signature_cleanup(UtObject *object) {
  UtDBusSignature *self = (UtDBusSignature *)object;
  free(self->value);
}

static UtObjectInterface object_interface = {
    .type_name = "UtDBusSignature",
    .init = ut_dbus_signature_init,
    .to_string = ut_dbus_signature_to_string,
    .cleanup = ut_dbus_signature_cleanup,
    .interfaces = {{NULL, NULL}}};

UtObject *ut_dbus_signature_new(const char *value) {
  UtObject *object = ut_object_new(sizeof(UtDBusSignature), &object_interface);
  UtDBusSignature *self = (UtDBusSignature *)object;
  self->value = strdup(value);
  return object;
}

const char *ut_dbus_signature_get_value(UtObject *object) {
  assert(ut_object_is_dbus_signature(object));
  UtDBusSignature *self = (UtDBusSignature *)object;
  return self->value;
}

static size_t get_signature_end(const char *signature, size_t offset) {
  if (signature[offset] == 'a') {
    return get_signature_end(signature, offset + 1);
  } else if (signature[offset] == '(') {
    size_t count = 1;
    size_t end = offset + 1;
    while (signature[end] != '\0') {
      if (signature[end] == ')') {
        count--;
        if (count == 0) {
          break;
        }
      } else if (signature[end] == '(') {
        count++;
      }
      end++;
    }
    return signature[end] == ')' ? end + 1 : 0;
  } else if (signature[offset] == '{') {
    size_t end = offset + 1;
    while (signature[end] != '\0' && signature[end] != '}') {
      end++;
    }
    return signature[end] == '}' ? end + 1 : 0;
  } else {
    return offset + 1;
  }
}

UtObject *ut_dbus_signature_split(UtObject *object) {
  assert(ut_object_is_dbus_signature(object));
  UtDBusSignature *self = (UtDBusSignature *)object;
  UtObjectRef signatures = ut_list_new();
  size_t offset = 0;
  while (self->value[offset] != '\0') {
    size_t end = get_signature_end(self->value, offset);
    if (end == 0) {
      return ut_general_error_new("Invalid DBus signature");
    }
    ut_cstring_ref signature = ut_cstring_substring(self->value, offset, end);
    ut_list_append_take(signatures, ut_dbus_signature_new(signature));
    offset = end;
  }
  return ut_object_ref(signatures);
}

bool ut_object_is_dbus_signature(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
