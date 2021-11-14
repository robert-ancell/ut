#include <assert.h>
#include <stdlib.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data) {
  assert(ut_list_get_length(data) == 1);
  assert(ut_uint32_list_get_element(data, 0) == 0x1f600);
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObjectRef sync_input =
      ut_uint8_list_new_with_data(4, 0xf0, 0x9f, 0x98, 0x80);
  UtObjectRef sync_input_stream = ut_list_input_stream_new(sync_input);
  UtObjectRef sync_decoder = ut_utf8_decoder_new(sync_input_stream);
  UtObjectRef sync_result = ut_input_stream_read_sync(sync_decoder);
  ut_assert_is_not_error(sync_result);
  assert(ut_list_get_length(sync_result) == 1);
  assert(ut_uint32_list_get_element(sync_result, 0) == 0x1f600);

  UtObjectRef async_input =
      ut_uint8_list_new_with_data(4, 0xf0, 0x9f, 0x98, 0x80);
  UtObjectRef async_input_stream = ut_list_input_stream_new(async_input);
  UtObjectRef async_decoder = ut_utf8_decoder_new(async_input_stream);
  ut_input_stream_read_all(async_decoder, read_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
