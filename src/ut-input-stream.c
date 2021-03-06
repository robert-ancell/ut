#include <assert.h>
#include <stdlib.h>

#include "ut-cancel.h"
#include "ut-error.h"
#include "ut-general-error.h"
#include "ut-input-stream.h"
#include "ut-list.h"

int ut_input_stream_id = 0;

typedef struct {
  UtObject *read_cancel;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} ReadAllData;

static ReadAllData *read_all_data_new(UtInputStreamCallback callback,
                                      void *user_data, UtObject *cancel) {
  ReadAllData *d = malloc(sizeof(ReadAllData));
  d->read_cancel = ut_cancel_new();
  d->callback = callback;
  d->user_data = user_data;
  d->cancel = ut_object_ref(cancel);
  return d;
}

static void read_all_data_free(ReadAllData *d) {
  ut_object_unref(d->read_cancel);
  ut_object_unref(d->cancel);
  free(d);
}

static size_t read_all_cb(void *user_data, UtObject *data, bool complete) {
  ReadAllData *d = user_data;

  if (ut_cancel_is_active(d->cancel)) {
    ut_cancel_activate(d->read_cancel);
    read_all_data_free(d);
    return 0;
  }

  if (ut_object_implements_error(data)) {
    d->callback(d->user_data, data, true);
    ut_cancel_activate(d->read_cancel);
    read_all_data_free(d);
    return 0;
  } else if (complete) {
    d->callback(d->user_data, data, true);
    read_all_data_free(d);
    return ut_list_get_length(data);
  } else {
    // Wait for all data.
    return 0;
  }
}

static size_t sync_cb(void *user_data, UtObject *data, bool complete) {
  UtObject **result = user_data;
  if (ut_object_implements_error(data)) {
    *result = ut_object_ref(data);
    return 0;
  } else if (complete) {
    assert(*result == NULL);
    *result = ut_list_copy(data);
    return ut_list_get_length(data);
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
  ReadAllData *d = read_all_data_new(callback, user_data, cancel);
  ut_input_stream_read(object, read_all_cb, d, d->read_cancel);
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

void ut_input_stream_set_active(UtObject *object, bool active) {
  UtInputStreamInterface *stream_interface =
      ut_object_get_interface(object, &ut_input_stream_id);
  assert(stream_interface != NULL);
  if (stream_interface->set_active != NULL) {
    stream_interface->set_active(object, active);
  }
}

bool ut_object_implements_input_stream(UtObject *object) {
  return ut_object_get_interface(object, &ut_input_stream_id) != NULL;
}
