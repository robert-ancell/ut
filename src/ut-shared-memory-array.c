#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-file-descriptor.h"
#include "ut-list.h"
#include "ut-output-stream.h"
#include "ut-shared-memory-array.h"
#include "ut-shared-memory-subarray.h"
#include "ut-string.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-uint8.h"

typedef struct {
  UtObject object;
  UtObject *fd;
  uint8_t *data;
  size_t data_length;
} UtSharedMemoryArray;

static UtObject *create_shared_memory() {
  char name[] = "/ut-XXXX";
  while (true) {
    for (size_t i = 4; i < 8; i++) {
      name[i] = '@' + (rand() & 0x3f);
    }
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd >= 0) {
      shm_unlink(name);
      return ut_file_descriptor_new(fd);
    }
    if (errno != EEXIST) {
      return NULL;
    }
  }
}

static uint8_t ut_shared_memory_array_get_element(UtObject *object,
                                                  size_t index) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  return self->data[index];
}

static uint8_t *ut_shared_memory_array_take_data(UtObject *object) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  uint8_t *result = malloc(self->data_length);
  memcpy(result, self->data, self->data_length);
  return result;
}

static size_t ut_shared_memory_array_get_length(UtObject *object) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  return self->data_length;
}

static UtObject *ut_shared_memory_array_get_element_object(UtObject *object,
                                                           size_t index) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  return ut_uint8_new(self->data[index]);
}

static UtObject *ut_shared_memory_array_get_sublist(UtObject *object,
                                                    size_t start,
                                                    size_t count) {
  return ut_shared_memory_subarray_new(object, start, count);
}

static UtObject *ut_shared_memory_array_copy(UtObject *object) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  UtObject *copy = ut_uint8_array_new();
  ut_uint8_list_append_block(copy, self->data, self->data_length);
  return copy;
}

static void ut_shared_memory_array_init(UtObject *object) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  self->fd = NULL;
  self->data = NULL;
  self->data_length = 0;
}

static char *ut_shared_memory_array_to_string(UtObject *object) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  return ut_cstring_new_printf("<UtSharedMemoryArray>(length: %zi)",
                               self->data_length);
}

static void ut_shared_memory_array_cleanup(UtObject *object) {
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  ut_object_unref(self->fd);
  munmap(self->data, self->data_length);
}

static UtUint8ListInterface uint8_list_interface = {
    .get_element = ut_shared_memory_array_get_element,
    .take_data = ut_shared_memory_array_take_data};

static UtListInterface list_interface = {
    .is_mutable = false,
    .get_length = ut_shared_memory_array_get_length,
    .get_element = ut_shared_memory_array_get_element_object,
    .get_sublist = ut_shared_memory_array_get_sublist,
    .copy = ut_shared_memory_array_copy};

static UtObjectInterface object_interface = {
    .type_name = "UtSharedMemoryArray",
    .init = ut_shared_memory_array_init,
    .to_string = ut_shared_memory_array_to_string,
    .cleanup = ut_shared_memory_array_cleanup,
    .interfaces = {{&ut_uint8_list_id, &uint8_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_shared_memory_array_new(size_t length) {
  UtObjectRef fd = create_shared_memory();
  return ut_shared_memory_array_new_from_fd(fd, length);
}

UtObject *ut_shared_memory_array_new_from_fd(UtObject *fd, size_t length) {
  UtObject *object =
      ut_object_new(sizeof(UtSharedMemoryArray), &object_interface);
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;

  self->fd = ut_object_ref(fd);

  ftruncate(ut_file_descriptor_get_fd(self->fd), length);
  self->data_length = length;

  self->data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED,
                    ut_file_descriptor_get_fd(self->fd), 0);

  return object;
}

UtObject *ut_shared_memory_array_get_fd(UtObject *object) {
  assert(ut_object_is_shared_memory_array(object));
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  return self->fd;
}

uint8_t *ut_shared_memory_array_get_data(UtObject *object) {
  assert(ut_object_is_shared_memory_array(object));
  UtSharedMemoryArray *self = (UtSharedMemoryArray *)object;
  return self->data;
}

bool ut_object_is_shared_memory_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
