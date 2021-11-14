#include <assert.h>

#include "ut-cancel.h"
#include "ut-end-of-stream.h"
#include "ut-input-stream.h"
#include "ut-list-input-stream.h"
#include "ut-list.h"

typedef struct {
  UtObject object;
  UtObject *data;
} UtListInputStream;

static void ut_list_input_stream_read(UtObject *object,
                                      UtInputStreamCallback callback,
                                      void *user_data, UtObject *cancel) {
  UtListInputStream *self = (UtListInputStream *)object;
  size_t data_length = ut_list_get_length(self->data);
  size_t n_used = callback(user_data, self->data);
  if (ut_cancel_is_active(cancel)) {
    return;
  }

  UtObjectRef unused_data =
      n_used == 0
          ? ut_object_ref(self->data)
          : ut_list_get_sublist(self->data, n_used, data_length - n_used);
  UtObjectRef eos = ut_end_of_stream_new(unused_data);
  callback(user_data, eos);
}

static void ut_list_input_stream_init(UtObject *object) {
  UtListInputStream *self = (UtListInputStream *)object;
  self->data = NULL;
}

static void ut_list_input_stream_cleanup(UtObject *object) {
  UtListInputStream *self = (UtListInputStream *)object;
  ut_object_unref(self->data);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_list_input_stream_read, .read_all = ut_list_input_stream_read};

static UtObjectInterface object_interface = {
    .type_name = "UtListInputStream",
    .init = ut_list_input_stream_init,
    .cleanup = ut_list_input_stream_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_list_input_stream_new(UtObject *list) {
  assert(list != NULL);
  UtObject *object =
      ut_object_new(sizeof(UtListInputStream), &object_interface);
  UtListInputStream *self = (UtListInputStream *)object;
  self->data = ut_object_ref(list);
  return object;
}

bool ut_object_is_list_input_stream(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
