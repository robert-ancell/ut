#include <assert.h>

#include "ut-cstring.h"
#include "ut-int32-array.h"
#include "ut-int32-list.h"
#include "ut-list.h"
#include "ut-uint8-array-with-fds.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  UtObject *data;
  UtObject *fds;
} UtUint8ArrayWithFds;

static uint8_t ut_uint8_array_with_fds_get_element(UtObject *object,
                                                   size_t index) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  return ut_uint8_list_get_element(self->data, index);
}

static uint8_t *ut_uint8_array_with_fds_take_data(UtObject *object) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  return ut_uint8_list_take_data(self->data);
}

static size_t ut_uint8_array_with_fds_get_length(UtObject *object) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  return ut_list_get_length(self->data);
}

static UtObject *ut_uint8_array_with_fds_get_element_object(UtObject *object,
                                                            size_t index) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  return ut_list_get_element(self->data, index);
}

static UtObject *ut_uint8_array_with_fds_get_sublist(UtObject *object,
                                                     size_t start,
                                                     size_t count) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  UtObjectRef data_sublist = ut_list_get_sublist(self->data, start, count);
  return ut_uint8_array_with_fds_new(data_sublist, self->fds);
}

static UtObject *ut_uint8_array_with_fds_copy(UtObject *object) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  UtObjectRef data_copy = ut_list_copy(self->data);
  UtObjectRef fds_copy = ut_list_copy(self->fds);
  return ut_uint8_array_with_fds_new(data_copy, fds_copy);
}

static void ut_uint8_array_with_fds_init(UtObject *object) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  self->data = NULL;
  self->fds = NULL;
}

static char *ut_uint8_array_with_fds_to_string(UtObject *object) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  ut_cstring_ref data_string = ut_object_to_string(self->data);
  ut_cstring_ref fds_string = ut_object_to_string(self->fds);
  return ut_cstring_new_printf("<UtUint8ArrayWithFds>(%s, %s)", data_string,
                               fds_string);
}

static void ut_uint8_array_with_fds_cleanup(UtObject *object) {
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  ut_object_unref(self->data);
  ut_object_unref(self->fds);
}

static UtUint8ListInterface uint8_list_interface = {
    .get_element = ut_uint8_array_with_fds_get_element,
    .take_data = ut_uint8_array_with_fds_take_data,
};

static UtListInterface list_interface = {
    .is_mutable = false,
    .get_length = ut_uint8_array_with_fds_get_length,
    .get_element = ut_uint8_array_with_fds_get_element_object,
    .get_sublist = ut_uint8_array_with_fds_get_sublist,
    .copy = ut_uint8_array_with_fds_copy};

static UtObjectInterface object_interface = {
    .type_name = "UtUint8ArrayWithFds",
    .init = ut_uint8_array_with_fds_init,
    .to_string = ut_uint8_array_with_fds_to_string,
    .cleanup = ut_uint8_array_with_fds_cleanup,
    .interfaces = {{&ut_uint8_list_id, &uint8_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_uint8_array_with_fds_new(UtObject *data, UtObject *fds) {
  assert(data != NULL);
  assert(ut_object_is_uint8_array(data));
  assert(fds != NULL);
  assert(ut_object_implements_int32_list(fds));
  UtObject *object =
      ut_object_new(sizeof(UtUint8ArrayWithFds), &object_interface);
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  self->data = ut_object_ref(data);
  self->fds = ut_object_ref(fds);
  return object;
}

UtObject *ut_uint8_array_with_fds_get_data(UtObject *object) {
  assert(ut_object_is_uint8_array_with_fds(object));
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  return self->data;
}

UtObject *ut_uint8_array_with_fds_get_fds(UtObject *object) {
  assert(ut_object_is_uint8_array_with_fds(object));
  UtUint8ArrayWithFds *self = (UtUint8ArrayWithFds *)object;
  return self->fds;
}

bool ut_object_is_uint8_array_with_fds(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
