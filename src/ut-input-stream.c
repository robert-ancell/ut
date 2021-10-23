#include <assert.h>

#include "ut-input-stream.h"
#include "ut-object-private.h"

int ut_input_stream_id = 0;

void ut_input_stream_read(UtObject *object, size_t block_size,
                          UtInputStreamCallback callback, void *user_data,
                          UtObject *cancel) {
  UtInputStreamInterface *stream_interface =
      ut_object_get_interface(object, &ut_input_stream_id);
  assert(stream_interface != NULL);
  stream_interface->read(object, block_size, callback, user_data, cancel);
}

void ut_input_stream_read_all(UtObject *object, size_t block_size,
                              UtInputStreamCallback callback, void *user_data,
                              UtObject *cancel) {
  UtInputStreamInterface *stream_interface =
      ut_object_get_interface(object, &ut_input_stream_id);
  assert(stream_interface != NULL);
  stream_interface->read_all(object, block_size, callback, user_data, cancel);
}

bool ut_object_implements_input_stream(UtObject *object) {
  return ut_object_get_interface(object, &ut_input_stream_id) != NULL;
}
