#include <assert.h>
#include <stdio.h>

#include "ut.h"

static UtObject *client = NULL;

static void event_cb(void *user_data, UtObject *event) {
  ut_cstring_ref s = ut_object_to_string(event);
  printf("%s\n", s);
}

static void error_cb(void *user_data, UtObject *error) {
  ut_cstring_ref s = ut_object_to_string(error);
  printf("%s\n", s);
}

static void intern_atom_cb(void *user_data, uint32_t atom, UtObject *error) {
  printf("Atom %08x\n", atom);
}

static void get_atom_name_cb(void *user_data, const char *name,
                             UtObject *error) {
  printf("Atom name \"%s\"\n", name);
}

static void list_extensions_cb(void *user_data, UtObject *names,
                               UtObject *error) {
  size_t names_length = ut_list_get_length(names);
  for (size_t i = 0; i < names_length; i++) {
    printf("%s\n", ut_string_list_get_element(names, i));
  }
}

static void connect_cb(void *user_data, UtObject *error) {
  if (error != NULL) {
    ut_cstring_ref description = ut_error_get_description(error);
    printf("Error connecting: %s", description);
    return;
  }

  printf("Connected\n");

  ut_x11_client_intern_atom(client, "HELLO-WORLD", true, intern_atom_cb, NULL,
                            NULL);

  ut_x11_client_get_atom_name(client, 0x00000001, get_atom_name_cb, NULL, NULL);

  ut_x11_client_list_extensions(client, list_extensions_cb, NULL, NULL);

  uint32_t window = ut_x11_client_create_window(client, 0, 0, 640, 480);
  ut_x11_client_map_window(client, window);
}

int main(int argc, char **argv) {
  client = ut_x11_client_new(event_cb, error_cb, NULL, NULL);
  ut_x11_client_connect(client, connect_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
