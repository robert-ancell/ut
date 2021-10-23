#include <stdio.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data) {
  printf("read - '%.*s'\n", (int)ut_list_get_length(data),
         ut_uint8_list_get_data(data));
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObjectRef readme = ut_local_file_new("README.md");
  ut_file_open_read(readme);
  ut_input_stream_read_all(readme, 1, read_cb, NULL, NULL);

  UtObjectRef test_file = ut_local_file_new("TEST");
  ut_file_open_write(test_file, true);
  UtObject *test_data = ut_constant_string_new("Hello\n");
  ut_output_stream_write_all(test_file, test_data, NULL, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
