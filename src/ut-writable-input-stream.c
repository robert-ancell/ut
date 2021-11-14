#include <assert.h>

#include "ut-cancel.h"
#include "ut-input-stream.h"
#include "ut-writable-input-stream.h"

typedef struct {
  UtObject object;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} UtWritableInputStream;

static void ut_writable_input_stream_init(UtObject *object) {
  UtWritableInputStream *self = (UtWritableInputStream *)object;
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
}

static void ut_writable_input_stream_cleanup(UtObject *object) {
  UtWritableInputStream *self = (UtWritableInputStream *)object;
  ut_object_unref(self->cancel);
}

static void ut_writable_input_stream_read(UtObject *object,
                                          UtInputStreamCallback callback,
                                          void *user_data, UtObject *cancel) {
  UtWritableInputStream *self = (UtWritableInputStream *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);

  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_writable_input_stream_read};

static UtObjectInterface object_interface = {
    .type_name = "UtWritableInputStream",
    .init = ut_writable_input_stream_init,
    .cleanup = ut_writable_input_stream_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_writable_input_stream_new() {
  return ut_object_new(sizeof(UtWritableInputStream), &object_interface);
}

size_t ut_writable_input_stream_write(UtObject *object, UtObject *data,
                                      bool complete) {
  assert(ut_object_is_writable_input_stream(object));
  UtWritableInputStream *self = (UtWritableInputStream *)object;

  assert(self->callback != NULL);
  return self->callback(self->user_data, data, complete);
}

bool ut_object_is_writable_input_stream(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
