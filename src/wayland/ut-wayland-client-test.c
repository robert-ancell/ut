#include <assert.h>
#include <stdio.h>

#include "ut.h"

static UtObject *client = NULL;

static void connect_cb(void *user_data, UtObject *error) {
  if (error != NULL) {
    ut_cstring_ref description = ut_error_get_description(error);
    printf("Error connecting: %s", description);
    return;
  }

  printf("Connected\n");

  uint32_t surface = ut_wayland_client_compositor_create_surface(client);
  uint32_t shell_surface =
      ut_wayland_client_shell_get_shell_surface(client, surface);
  ut_wayland_client_shell_surface_set_toplevel(client, shell_surface);
  ut_wayland_client_shell_surface_set_title(client, shell_surface,
                                            "Test Window");
}

int main(int argc, char **argv) {
  client = ut_wayland_client_new();
  ut_wayland_client_connect(client, connect_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
