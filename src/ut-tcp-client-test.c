#include <stdio.h>

#include "ut.h"

static size_t http_read_cb(void *user_data, UtObject *data) {
  UtObjectRef text = ut_string_new_from_utf8(data);
  printf("http read:\n%s'\n", ut_string_get_text(text));
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

static void http_connect_cb(void *user_data) {
  UtObject *tcp_client = user_data;

  printf("http connect\n");
  UtObjectRef request =
      ut_string_new_constant("GET / HTTP/1.1\nHost: example.com\n\n");
  UtObjectRef utf8 = ut_string_get_utf8(request);
  ut_output_stream_write_all(tcp_client, utf8, NULL, NULL, NULL);
  ut_input_stream_read(tcp_client, http_read_cb, NULL, NULL);
}

int main(int argc, char **argv) {
  UtObjectRef tcp_client = ut_tcp_client_new("example.com", 80);
  ut_tcp_client_connect(tcp_client, http_connect_cb, tcp_client, NULL);

  ut_event_loop_run();

  return 0;
}
