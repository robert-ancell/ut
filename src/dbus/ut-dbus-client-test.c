#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut.h"

static UtObject *client = NULL;

static void capabilities_cb(void *user_data, UtObject *out_args) {
  assert(ut_list_get_length(out_args) == 1);
  ut_assert_is_not_error(out_args);
  UtObjectRef capabilities = ut_list_get_element(out_args, 0);
  assert(ut_object_is_dbus_array(capabilities));
  assert(strcmp(ut_dbus_array_get_value_signature(capabilities), "s") == 0);
  size_t capabilities_length = ut_list_get_length(capabilities);
  printf("Capabilities:");
  for (size_t i = 0; i < capabilities_length; i++) {
    UtObjectRef capability_object = ut_list_get_element(capabilities, i);
    printf(" %s", ut_string_get_text(capability_object));
  }
  printf("\n");
}

static void notify_cb(void *user_data, UtObject *out_args) {
  ut_cstring_ref text = ut_object_to_string(out_args);
  ut_assert_is_not_error(out_args);
  assert(ut_list_get_length(out_args) == 1);
  UtObjectRef id_object = ut_list_get_element(out_args, 0);
  assert(ut_object_is_uint32(id_object));
  printf("Id: %d\n", ut_uint32_get_value(id_object));

  ut_event_loop_return(NULL);
}

int main(int argc, char **argv) {
  client = ut_dbus_client_new_session();

  ut_dbus_client_call_method(client, "org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications", "GetCapabilities",
                             NULL, capabilities_cb, NULL, NULL);

  UtObjectRef args = ut_list_new_with_elements_take(
      ut_string_new(""), ut_uint32_new(0), ut_string_new(""),
      ut_string_new("Hello World!"), ut_string_new(""), ut_dbus_array_new("s"),
      ut_dbus_dict_new("s", "v"), ut_int32_new(-1), NULL);
  ut_dbus_client_call_method(
      client, "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
      "org.freedesktop.Notifications", "Notify", args, notify_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
