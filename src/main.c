#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-file.h"
#include "ut-immutable-utf8-string.h"
#include "ut-list.h"
#include "ut-mutable-string.h"
#include "ut-mutable-uint8-list.h"
#include "ut-mutable-utf8-string.h"
#include "ut-string.h"
#include "ut-uint32-list.h"
#include "ut-uint8-list.h"
#include "ut-utf8-string.h"

static void delay2_cb(void *user_data) { printf("delay 2s\n"); }

static void delay3_cb(void *user_data) { printf("delay 3s\n"); }

static void delay5_cb(void *user_data) {
  UtObject *cancel = user_data;
  printf("delay 5s\n");
  ut_cancel_activate(cancel);
}

static void timer_cb(void *user_data) { printf("timer\n"); }

static void stdin_cb(void *user_data) {
  char buffer[1024];
  ssize_t n_read = read(0, buffer, 1024);
  printf("stdin - '%.*s'\n", (int)n_read, buffer);
}

static void read_cb(void *user_data, UtObject *data) {
  printf("read - '%.*s'\n", (int)ut_list_get_length(data),
         ut_uint8_list_get_data(data));
}

int main(int argc, char **argv) {
  UtObject *list = ut_mutable_uint8_list_new();
  ut_mutable_uint8_list_append(list, 0x01);
  ut_mutable_uint8_list_append(list, 0x02);
  ut_mutable_uint8_list_append(list, 0x03);
  printf("%zi\n", ut_list_get_length(list));
  printf("%02X\n", ut_uint8_list_get_data(list)[1]);
  printf("%s\n", ut_object_get_type_name(list));
  ut_object_unref(list);

  UtObject *readme = ut_file_new("README.md");
  ut_file_open_read(readme);
  ut_file_read_all(readme, 1, read_cb, NULL, NULL);

  UtObject *string = ut_immutable_utf8_string_new("Hello");
  UtObject *code_points = ut_string_get_code_points(string);
  const uint32_t *code_point_data = ut_uint32_list_get_data(code_points);
  printf("code points:");
  for (size_t i = 0; i < ut_list_get_length(code_points); i++) {
    printf(" %x", code_point_data[i]);
  }
  printf("\n");

  UtObject *test_file = ut_file_new("TEST");
  ut_file_open_write(test_file, true);
  UtObject *test_data = ut_immutable_utf8_string_new("Hello\n");
  ut_file_write_all(test_file, test_data, NULL, NULL, NULL);

  UtObject *string2 = ut_mutable_utf8_string_new(" ");
  printf("'%s'\n", ut_utf8_string_get_text(string2));
  ut_mutable_string_prepend(string2, "Hello");
  printf("'%s'\n", ut_utf8_string_get_text(string2));
  ut_mutable_string_append(string2, "World!");
  printf("'%s'\n", ut_utf8_string_get_text(string2));

  UtObject *loop = ut_event_loop_get();

  UtObject *timer_cancel = ut_cancel_new();
  ut_event_loop_add_delay(loop, 2, delay2_cb, NULL, NULL);
  ut_event_loop_add_delay(loop, 5, delay5_cb, timer_cancel, NULL);
  ut_event_loop_add_delay(loop, 3, delay3_cb, NULL, NULL);
  ut_event_loop_add_timer(loop, 1, timer_cb, NULL, timer_cancel);

  ut_event_loop_add_read_watch(loop, 0, stdin_cb, NULL, NULL);

  ut_event_loop_run(loop);

  ut_object_unref(readme);
  ut_object_unref(test_file);
  ut_object_unref(timer_cancel);

  return 0;
}
