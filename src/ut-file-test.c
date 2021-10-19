#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data) {
  printf("read - '%.*s'\n", (int)ut_list_get_length(data),
         ut_uint8_list_get_data(data));
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObject *readme = ut_file_new("README.md");
  ut_file_open_read(readme);
  ut_file_read_all(readme, 1, read_cb, NULL, NULL);

  UtObject *test_file = ut_file_new("TEST");
  ut_file_open_write(test_file, true);
  UtObject *test_data = ut_constant_string_new("Hello\n");
  ut_output_stream_write_all(test_file, test_data, NULL, NULL, NULL);

  ut_event_loop_run();

  ut_object_unref(readme);
  ut_object_unref(test_file);

  return 0;
}
