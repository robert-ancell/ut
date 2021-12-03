#include <assert.h>
#include <stdio.h>

#include "ut.h"

static UtObject *client = NULL;

static void ping_cb(void *user_data, UtObject *out_args) {
  if (ut_object_implements_error(out_args)) {
    ut_cstring_ref text = ut_error_get_description(out_args);
    printf("%s\n", text);
    return;
  }

  ut_cstring_ref text = ut_object_to_string(out_args);
  printf("%s\n", text);

  ut_event_loop_return(NULL);
}

int main(int argc, char **argv) {
  client = ut_dbus_client_new_session();
  UtObjectRef args = ut_list_new_with_data_take(
      ut_string_new(""), ut_uint32_new(0), ut_string_new(""),
      ut_string_new("Hello World!"), ut_string_new(""), ut_dbus_array_new("s"),
      ut_dbus_dict_new("s", "v"), ut_int32_new(-1), NULL);
  ut_dbus_client_call_method(
      client, "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
      "org.freedesktop.Notifications", "Notify", args, ping_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
