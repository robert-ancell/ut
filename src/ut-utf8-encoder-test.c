#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data) {
  ut_cstring d = ut_object_to_string(data);
  printf("%s\n", d);
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObjectRef code_points = ut_uint32_array_new();
  ut_uint32_array_append(code_points, 'H');
  ut_uint32_array_append(code_points, 'i');
  ut_uint32_array_append(code_points, 0x1f600);
  UtObjectRef utf8_encoder = ut_utf8_encoder_new(code_points);
  ut_input_stream_read_all(utf8_encoder, read_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
