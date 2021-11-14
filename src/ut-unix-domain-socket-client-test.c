#include <stdio.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtObjectRef text = ut_string_new_from_utf8(data);
  printf("read:\n%s'\n", ut_string_get_text(text));
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObjectRef client = ut_unix_domain_socket_client_new("/run/snapd.socket");
  ut_unix_domain_socket_client_connect(client);

  UtObjectRef request = ut_string_new_constant("GET / HTTP/1.1\nHost:\n\n");
  UtObjectRef utf8 = ut_string_get_utf8(request);
  ut_output_stream_write(client, utf8);
  ut_input_stream_read(client, read_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
