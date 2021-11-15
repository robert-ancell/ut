#include <assert.h>

#include "ut-cancel.h"
#include "ut-error.h"
#include "ut-input-stream-multiplexer.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-writable-input-stream.h"

typedef struct {
  UtObject object;
  UtObject *input_stream;
  UtObject *streams;
  UtObject *active_stream;
  UtObject *cancel;
  bool reading;
  bool stream_changed;
} UtInputStreamMultiplexer;

static bool is_multiplexer_stream(UtInputStreamMultiplexer *self,
                                  UtObject *stream) {
  size_t streams_length = ut_list_get_length(self->streams);
  for (size_t i = 0; i < streams_length; i++) {
    if (stream == ut_list_get_element(self->streams, i)) {
      return true;
    }
  }

  return false;
}

static size_t read_cb(void *user_data, UtObject *data, bool complete);

static void start_read(UtInputStreamMultiplexer *self) {
  if (self->reading) {
    return;
  }
  self->reading = true;
  ut_input_stream_read(self->input_stream, read_cb, self, self->cancel);
}

static void reading_cb(void *user_data, UtObject *stream) {
  UtInputStreamMultiplexer *self = user_data;
  if (stream == self->active_stream) {
    start_read(self);
    ut_input_stream_set_active(self->input_stream, true);
  }
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtInputStreamMultiplexer *self = user_data;

  if (ut_object_implements_error(data)) {
    return ut_writable_input_stream_write(self->active_stream, data, complete);
  }

  size_t total_used = 0;
  size_t data_length = ut_list_get_length(data);

  do {
    self->stream_changed = false;

    UtObjectRef d =
        total_used == 0
            ? ut_object_ref(data)
            : ut_list_get_sublist(data, total_used, data_length - total_used);
    size_t n_used =
        ut_writable_input_stream_write(self->active_stream, d, complete);
    total_used += n_used;
  } while (self->stream_changed &&
           ut_writable_input_stream_get_reading(self->active_stream));

  return total_used;
}

static void ut_input_stream_multiplexer_init(UtObject *object) {
  UtInputStreamMultiplexer *self = (UtInputStreamMultiplexer *)object;
  self->input_stream = NULL;
  self->streams = ut_list_new();
  self->active_stream = NULL;
  self->cancel = ut_cancel_new();
  self->reading = false;
  self->stream_changed = false;
}

static void ut_input_stream_multiplexer_cleanup(UtObject *object) {
  UtInputStreamMultiplexer *self = (UtInputStreamMultiplexer *)object;
  ut_cancel_activate(self->cancel);
  ut_object_unref(self->input_stream);
  ut_object_unref(self->streams);
  ut_object_unref(self->cancel);
}

static UtObjectInterface object_interface = {
    .type_name = "UtInputStreamMultiplexer",
    .init = ut_input_stream_multiplexer_init,
    .cleanup = ut_input_stream_multiplexer_cleanup,
    .interfaces = {{NULL, NULL}}};

UtObject *ut_input_stream_multiplexer_new(UtObject *input_stream) {
  UtObject *object =
      ut_object_new(sizeof(UtInputStreamMultiplexer), &object_interface);
  UtInputStreamMultiplexer *self = (UtInputStreamMultiplexer *)object;
  self->input_stream = ut_object_ref(input_stream);
  return object;
}

UtObject *ut_input_stream_multiplexer_add(UtObject *object) {
  assert(ut_object_is_input_stream_multiplexer(object));
  UtInputStreamMultiplexer *self = (UtInputStreamMultiplexer *)object;
  UtObject *stream = ut_writable_input_stream_new();
  ut_writable_input_stream_set_reading_callback(stream, reading_cb, self,
                                                self->cancel);
  ut_list_append(self->streams, stream);
  return stream;
}

void ut_input_stream_multiplexer_set_active(UtObject *object,
                                            UtObject *stream) {
  assert(ut_object_is_input_stream_multiplexer(object));
  UtInputStreamMultiplexer *self = (UtInputStreamMultiplexer *)object;
  assert(is_multiplexer_stream(self, stream));

  bool first_stream = self->active_stream == NULL;
  self->stream_changed = true;
  self->active_stream = stream;
  if (first_stream) {
    if (ut_writable_input_stream_get_reading(stream)) {
      start_read(self);
    }
  } else {
    ut_input_stream_set_active(self->input_stream,
                               ut_writable_input_stream_get_reading(stream));
  }
}

bool ut_object_is_input_stream_multiplexer(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
