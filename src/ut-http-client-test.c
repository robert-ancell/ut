#include <stdio.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data) {
  UtObjectRef text = ut_string_new_from_utf8(data);
  printf("http read:\n%s'\n", ut_string_get_text(text));
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

static void http_response_cb(void *user_data, UtObject *response) {
  printf("%d '%s'\n", ut_http_response_get_status_code(response),
         ut_http_response_get_reason_phrase(response));
  UtObject *headers = ut_http_response_get_headers(response);
  for (size_t i = 0; i < ut_list_get_length(headers); i++) {
    UtObject *header = ut_object_list_get_element(headers, i);
    printf("%s: %s\n", ut_http_header_get_name(header),
           ut_http_header_get_value(header));
  }
  ut_input_stream_read_all(response, read_cb, NULL, NULL);
}

int main(int argc, char **argv) {
  UtObjectRef http_client = ut_http_client_new();
  ut_http_client_send_request(http_client, "GET", "http://example.com",
                              http_response_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
