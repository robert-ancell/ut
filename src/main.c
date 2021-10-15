#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut.h"

static void delay2_cb(void *user_data) { printf("delay 2s\n"); }

static void delay3_cb(void *user_data) { printf("delay 3s\n"); }

static void delay5_cb(void *user_data) {
  UtObject *cancel = user_data;
  printf("delay 5s\n");
  ut_cancel_activate(cancel);
}

static void timer_cb(void *user_data) { printf("timer\n"); }

static void *thread_cb(void *data) {
  sleep(2);
  return strdup("Hello World");
}

static void thread_result_cb(void *user_data, void *result) {
  char *result_ = result;
  printf("Thread result: '%s'\n", result_);
  free(result_);
}

static void stdin_cb(void *user_data) {
  char buffer[1024];
  ssize_t n_read = read(0, buffer, 1024);
  printf("stdin - '%.*s'\n", (int)n_read, buffer);
}

static void read_cb(void *user_data, UtObject *data) {
  printf("read - '%.*s'\n", (int)ut_list_get_length(data),
         ut_uint8_list_get_data(data));
}

static void http_read_cb(void *user_data, UtObject *data) {
  printf("http read:\n%.*s'\n", (int)ut_list_get_length(data),
         ut_uint8_list_get_data(data));
}

static void http_connect_cb(void *user_data) {
  UtObject *tcp_client = user_data;

  printf("http connect\n");
  UtObject *request =
      ut_immutable_string_new("GET / HTTP/1.1\nHost: example.com\n\n");
  ut_tcp_client_write_all(tcp_client, request, NULL, NULL, NULL);
  ut_tcp_client_read(tcp_client, 65535, http_read_cb, NULL, NULL);
}

int main(int argc, char **argv) {
  UtObject *list = ut_mutable_uint8_list_new();
  ut_mutable_uint8_list_append(list, 0x01);
  ut_mutable_uint8_list_append(list, 0x02);
  ut_mutable_uint8_list_append(list, 0x03);
  printf("%zi\n", ut_list_get_length(list));
  printf("%02X\n", ut_uint8_list_get_data(list)[1]);
  printf("%s\n", ut_object_get_type_name(list));
  char *list_string = ut_object_to_string(list);
  printf("list: %s\n", list_string);
  free(list_string);
  ut_object_unref(list);

  UtObject *map = ut_hash_map_new();
  ut_map_insert_string_take(map, "one", ut_uint8_new(1));
  ut_map_insert_string_take(map, "two", ut_uint8_new(42));
  ut_map_insert_string_take(map, "two", ut_uint8_new(2));
  char *map_string = ut_object_to_string(map);
  printf("map: %s\n", map_string);
  free(map_string);

  UtObject *readme = ut_file_new("README.md");
  ut_file_open_read(readme);
  ut_file_read_all(readme, 1, read_cb, NULL, NULL);

  UtObject *string = ut_immutable_string_new("Hello");
  UtObject *code_points = ut_string_get_code_points(string);
  const uint32_t *code_point_data = ut_uint32_list_get_data(code_points);
  printf("code points:");
  for (size_t i = 0; i < ut_list_get_length(code_points); i++) {
    printf(" %x", code_point_data[i]);
  }
  printf("\n");

  UtObject *test_file = ut_file_new("TEST");
  ut_file_open_write(test_file, true);
  UtObject *test_data = ut_immutable_string_new("Hello\n");
  ut_file_write_all(test_file, test_data, NULL, NULL, NULL);

  UtObject *tcp_client = ut_tcp_client_new("example.com", 80);
  ut_tcp_client_connect(tcp_client, http_connect_cb, tcp_client, NULL);

  UtObject *string2 = ut_mutable_string_new(" ");
  printf("'%s'\n", ut_string_get_text(string2));
  ut_mutable_string_prepend(string2, "Hello");
  printf("'%s'\n", ut_string_get_text(string2));
  ut_mutable_string_append(string2, "World!");
  printf("'%s'\n", ut_string_get_text(string2));

  UtObject *timer_cancel = ut_cancel_new();
  ut_event_loop_add_delay(2, delay2_cb, NULL, NULL);
  ut_event_loop_add_delay(5, delay5_cb, timer_cancel, NULL);
  ut_event_loop_add_delay(3, delay3_cb, NULL, NULL);
  ut_event_loop_add_timer(1, timer_cb, NULL, timer_cancel);

  ut_event_loop_run_in_thread(thread_cb, NULL, NULL, thread_result_cb, NULL,
                              NULL);

  ut_event_loop_add_read_watch(0, stdin_cb, NULL, NULL);

  ut_event_loop_run();

  ut_object_unref(readme);
  ut_object_unref(test_file);
  ut_object_unref(timer_cancel);

  return 0;
}
