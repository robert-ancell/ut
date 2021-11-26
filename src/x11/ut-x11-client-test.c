#include <assert.h>
#include <stdio.h>

#include "ut.h"

static size_t width = 640;
static size_t height = 480;

static UtObject *client = NULL;
static uint32_t segment = 0;
static UtObject *buffer = NULL;
static uint32_t window = 0;
static uint32_t pixmap = 0;
static uint32_t gc = 0;

static void event_cb(void *user_data, UtObject *event) {
  ut_cstring_ref s = ut_object_to_string(event);
  printf("%s\n", s);

  if (ut_object_is_x11_expose(event)) {
    ut_x11_client_copy_area(client, pixmap, window, gc, 0, 0, 0, 0, width,
                            height);
  }
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

static void create_segment_cb(void *user_data, UtObject *fd, UtObject *error) {
  buffer = ut_shared_memory_array_new_from_fd(fd);
  uint8_t *pixmap_data = ut_shared_memory_array_get_data(buffer);
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      uint8_t *pixel = pixmap_data + (y * width * 4) + (x * 4);
      pixel[0] = 255 * x / width;
      pixel[1] = 255 * y / height;
      pixel[2] = 255;
      pixel[3] = 255;
    }
  }

  UtObject *shm = ut_x11_client_get_mit_shm_extension(client);
  pixmap = ut_x11_mit_shm_extension_create_pixmap(shm, window, width, height,
                                                  24, segment, 0);
  gc = ut_x11_client_create_gc(client, pixmap);
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

  window = ut_x11_client_create_window(client, 0, 0, width, height);
  ut_x11_client_map_window(client, window);

  UtObject *shm = ut_x11_client_get_mit_shm_extension(client);
  segment = ut_x11_mit_shm_extension_create_segment(
      shm, width * height * 4, false, create_segment_cb, NULL, NULL);
}

int main(int argc, char **argv) {
  client = ut_x11_client_new(event_cb, error_cb, NULL, NULL);
  ut_x11_client_connect(client, connect_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
