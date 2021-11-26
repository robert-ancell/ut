#include <assert.h>

#include "ut-x11-error.h"
#include "ut-x11-shm-segment-error.h"

typedef struct {
  UtObject object;
  uint32_t segment;
} UtX11ShmSegmentError;

static void ut_x11_shm_segment_error_init(UtObject *object) {
  UtX11ShmSegmentError *self = (UtX11ShmSegmentError *)object;
  self->segment = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11ShmSegmentError",
    .init = ut_x11_shm_segment_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_shm_segment_error_new(uint32_t segment) {
  UtObject *object =
      ut_object_new(sizeof(UtX11ShmSegmentError), &object_interface);
  UtX11ShmSegmentError *self = (UtX11ShmSegmentError *)object;
  self->segment = segment;
  return object;
}

uint32_t ut_x11_shm_segment_error_get_segment(UtObject *object) {
  assert(ut_object_is_x11_shm_segment_error(object));
  UtX11ShmSegmentError *self = (UtX11ShmSegmentError *)object;
  return self->segment;
}

bool ut_object_is_x11_shm_segment_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
