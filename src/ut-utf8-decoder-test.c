#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data) {
  assert(ut_list_get_length(data) == 1);
  assert(ut_uint32_list_get_element(data, 0) == 0x1f600);
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObjectRef sync_input = ut_uint8_array_new();
  ut_uint8_array_append(sync_input, 0xf0);
  ut_uint8_array_append(sync_input, 0x9f);
  ut_uint8_array_append(sync_input, 0x98);
  ut_uint8_array_append(sync_input, 0x80);
  UtObjectRef sync_decoder = ut_utf8_decoder_new(sync_input);
  UtObjectRef sync_result = ut_input_stream_read_sync(sync_decoder);
  assert(ut_list_get_length(sync_result) == 1);
  assert(ut_uint32_list_get_element(sync_result, 0) == 0x1f600);

  UtObjectRef async_input = ut_uint8_array_new();
  ut_uint8_array_append(async_input, 0xf0);
  ut_uint8_array_append(async_input, 0x9f);
  ut_uint8_array_append(async_input, 0x98);
  ut_uint8_array_append(async_input, 0x80);
  UtObjectRef async_decoder = ut_utf8_decoder_new(async_input);
  ut_input_stream_read_all(async_decoder, read_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
