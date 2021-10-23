#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data) {
  char *code_points = ut_object_to_string(data);
  printf("%s\n", code_points);
  free(code_points);
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObjectRef readme = ut_local_file_new("README.md");
  ut_file_open_read(readme);
  UtObjectRef utf8_decoder = ut_utf8_decoder_new(readme);
  ut_input_stream_read_all(utf8_decoder, 1, read_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
