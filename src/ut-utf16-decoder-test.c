#include <assert.h>
#include <stdlib.h>

#include "ut.h"

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  assert(ut_list_get_length(data) == 1);
  assert(ut_uint32_list_get_element(data, 0) == 0x1f600);
  ut_event_loop_return(NULL);
  return ut_list_get_length(data);
}

int main(int argc, char **argv) {
  UtObjectRef sync_input = ut_uint16_list_new_with_data(2, 0xd83d, 0xde00);
  UtObjectRef sync_input_stream = ut_list_input_stream_new(sync_input);
  UtObjectRef sync_decoder = ut_utf16_decoder_new(sync_input_stream);
  UtObjectRef sync_result = ut_input_stream_read_sync(sync_decoder);
  ut_assert_is_not_error(sync_result);
  printf("%s\n", ut_object_to_string(sync_result));
  assert(ut_list_get_length(sync_result) == 1);
  assert(ut_uint32_list_get_element(sync_result, 0) == 0x1f600);

  UtObjectRef async_input = ut_uint16_list_new_with_data(2, 0xd83d, 0xde00);
  UtObjectRef async_input_stream = ut_list_input_stream_new(async_input);
  UtObjectRef async_decoder = ut_utf16_decoder_new(async_input_stream);
  ut_input_stream_read_all(async_decoder, read_cb, NULL, NULL);

  ut_event_loop_run();

  return 0;
}
