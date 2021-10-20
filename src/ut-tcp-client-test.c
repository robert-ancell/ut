#include <stdio.h>

#include "ut.h"

static size_t http_read_cb(void *user_data, UtObject *data) {
  printf("http read:\n%.*s'\n", (int)ut_list_get_length(data),
         ut_uint8_list_get_data(data));
  return ut_list_get_length(data);
}

static void http_connect_cb(void *user_data) {
  UtObject *tcp_client = user_data;

  printf("http connect\n");
  UtObject *request =
      ut_constant_string_new("GET / HTTP/1.1\nHost: example.com\n\n");
  ut_output_stream_write_all(tcp_client, request, NULL, NULL, NULL);
  ut_input_stream_read(tcp_client, 65535, http_read_cb, NULL, NULL);
}

int main(int argc, char **argv) {
  UtObjectRef tcp_client = ut_tcp_client_new("example.com", 80);
  ut_tcp_client_connect(tcp_client, http_connect_cb, tcp_client, NULL);

  ut_event_loop_run();

  return 0;
}
