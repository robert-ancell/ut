#include <assert.h>
#include <stdio.h>

#include "ut.h"

static UtObject *client = NULL;

static void intern_atom_cb(void *user_data, uint32_t atom) {
  printf("Atom %08x\n", atom);
}

static void get_atom_name_cb(void *user_data, const char *name) {
  printf("Atom name \"%s\"\n", name);
}

static void connect_cb(void *user_data, UtObject *error) {
  if (error != NULL) {
    ut_cstring description = ut_error_get_description(error);
    printf("Error connecting: %s", description);
    return;
  }

  printf("Connected\n");

  ut_x11_client_intern_atom(client, "HELLO-WORLD", true, intern_atom_cb, NULL,
                            NULL);

  ut_x11_client_get_atom_name(client, 0x00000001, get_atom_name_cb, NULL, NULL);

  uint32_t window = ut_x11_client_create_window(client, 0, 0, 640, 480);
  ut_x11_client_map_window(client, window);
}

int main(int argc, char **argv) {
  client = ut_x11_client_new();
  ut_x11_client_connect(client, connect_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
