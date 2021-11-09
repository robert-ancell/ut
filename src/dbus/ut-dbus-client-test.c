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
}

int main(int argc, char **argv) {
  client = ut_dbus_client_new_system();
  ut_dbus_client_call_method(
      client, "org.freedesktop.DBus", "/org/freedesktop/DBus",
      "org.freedesktop.DBus.Peer", "Ping", NULL, ping_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
