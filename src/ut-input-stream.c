#include <assert.h>

#include "ut-cancel.h"
#include "ut-end-of-stream.h"
#include "ut-error.h"
#include "ut-general-error.h"
#include "ut-input-stream.h"
#include "ut-list.h"

int ut_input_stream_id = 0;

static size_t sync_cb(void *user_data, UtObject *data) {
  UtObject **result = user_data;
  if (ut_object_is_end_of_stream(data)) {
    UtObject *unused_data = ut_end_of_stream_get_unused_data(data);
    *result = ut_list_copy(unused_data);
    return ut_list_get_length(unused_data);
  } else if (ut_object_implements_error(data)) {
    *result = ut_object_ref(data);
    return 0;
  } else {
    // Wait for all data.
    return 0;
  }
}

void ut_input_stream_read(UtObject *object, UtInputStreamCallback callback,
                          void *user_data, UtObject *cancel) {
  UtInputStreamInterface *stream_interface =
      ut_object_get_interface(object, &ut_input_stream_id);
  assert(stream_interface != NULL);
  stream_interface->read(object, callback, user_data, cancel);
}

void ut_input_stream_read_all(UtObject *object, UtInputStreamCallback callback,
                              void *user_data, UtObject *cancel) {
  UtInputStreamInterface *stream_interface =
      ut_object_get_interface(object, &ut_input_stream_id);
  assert(stream_interface != NULL);
  stream_interface->read_all(object, callback, user_data, cancel);
}

UtObject *ut_input_stream_read_sync(UtObject *object) {
  UtObject *result = NULL;
  UtObjectRef cancel = ut_cancel_new();
  ut_input_stream_read(object, sync_cb, &result, cancel);
  ut_cancel_activate(cancel);
  if (result == NULL) {
    result = ut_general_error_new("Sync call did not complete");
  }
  return result;
}

bool ut_object_implements_input_stream(UtObject *object) {
  return ut_object_get_interface(object, &ut_input_stream_id) != NULL;
}
