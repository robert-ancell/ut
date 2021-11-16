#include <stdio.h>

#include "ut.h"

static void png_cb(void *user_data, UtObject *image) {
  printf("%s\n", ut_object_to_string(image));
  ut_event_loop_return(NULL);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Need filename to decode\n");
    return 1;
  }

  UtObjectRef file = ut_local_file_new(argv[1]);
  ut_file_open_read(file);
  UtObjectRef decoder = ut_png_decoder_new(file);
  ut_png_decoder_decode(decoder, png_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
