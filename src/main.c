#include <stdint.h>
#include <stdio.h>

#include "ut-event-loop.h"
#include "ut-list.h"
#include "ut-mutable-uint8-list.h"
#include "ut-uint8-list.h"

static void timeout2_cb(void *user_data) { printf("timeout 2s\n"); }

static void timeout3_cb(void *user_data) { printf("timeout 3s\n"); }

static void timeout5_cb(void *user_data) { printf("timeout 5s\n"); }

int main(int argc, char **argv) {
  UtObject *list = ut_mutable_uint8_list_new();
  ut_mutable_uint8_list_append(list, 0x01);
  ut_mutable_uint8_list_append(list, 0x02);
  ut_mutable_uint8_list_append(list, 0x03);
  printf("%zi\n", ut_list_get_length(list));
  printf("%02X\n", ut_uint8_list_get_data(list)[1]);
  printf("%s\n", ut_object_get_type_name(list));
  ut_object_unref(list);

  UtObject *loop = ut_event_loop_new();
  ut_event_loop_add_timeout(loop, 2, timeout2_cb, NULL);
  ut_event_loop_add_timeout(loop, 5, timeout5_cb, NULL);
  ut_event_loop_add_timeout(loop, 3, timeout3_cb, NULL);
  ut_event_loop_run(loop);

  return 0;
}
