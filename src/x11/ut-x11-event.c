#include <assert.h>

#include "ut-x11-event.h"

int ut_x11_event_id = 0;

bool ut_object_implements_x11_event(UtObject *object) {
  return ut_object_get_interface(object, &ut_x11_event_id) != NULL;
}
