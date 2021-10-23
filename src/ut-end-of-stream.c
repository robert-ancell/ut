#include <assert.h>

#include "ut-end-of-stream.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  UtObject *unused_data;
} UtEndOfStream;

static void ut_end_of_stream_init(UtObject *object) {
  UtEndOfStream *self = (UtEndOfStream *)object;
  self->unused_data = NULL;
}

static void ut_end_of_stream_cleanup(UtObject *object) {
  UtEndOfStream *self = (UtEndOfStream *)object;
  if (self->unused_data != NULL) {
    ut_object_unref(self->unused_data);
  }
}

static UtObjectFunctions object_functions = {.type_name = "UtEndOfStream",
                                             .init = ut_end_of_stream_init,
                                             .cleanup =
                                                 ut_end_of_stream_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_end_of_stream_new(UtObject *unused_data) {
  UtObject *object = ut_object_new(sizeof(UtEndOfStream), &object_functions);
  UtEndOfStream *self = (UtEndOfStream *)object;
  self->unused_data = unused_data != NULL ? ut_object_ref(unused_data) : NULL;
  return object;
}

UtObject *ut_end_of_stream_get_unused_data(UtObject *object) {
  assert(ut_object_is_end_of_stream(object));
  UtEndOfStream *self = (UtEndOfStream *)object;
  return self->unused_data;
}

bool ut_object_is_end_of_stream(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
