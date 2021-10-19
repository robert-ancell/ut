#include <assert.h>

#include "ut-object-private.h"
#include "ut-output-stream.h"

int ut_output_stream_id = 0;

void ut_output_stream_write(UtObject *object, UtObject *data,
                            UtOutputStreamCallback callback, void *user_data,
                            UtObject *cancel) {
  UtOutputStreamFunctions *stream_functions =
      ut_object_get_interface(object, &ut_output_stream_id);
  assert(stream_functions != NULL);
  stream_functions->write(object, data, callback, user_data, cancel);
}

void ut_output_stream_write_all(UtObject *object, UtObject *data,
                                UtOutputStreamCallback callback,
                                void *user_data, UtObject *cancel) {
  UtOutputStreamFunctions *stream_functions =
      ut_object_get_interface(object, &ut_output_stream_id);
  assert(stream_functions != NULL);
  stream_functions->write_all(object, data, callback, user_data, cancel);
}

bool ut_object_implements_output_stream(UtObject *object) {
  return ut_object_get_interface(object, &ut_output_stream_id) != NULL;
}
