#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "ut-file-descriptor.h"
#include "ut-file.h"
#include "ut-list.h"
#include "ut-local-file.h"
#include "ut-memory-mapped-file.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-uint8.h"

typedef struct {
  UtObject object;
  UtObject *file;
  uint8_t *data;
  size_t data_length;
} UtMemoryMappedFile;

static void map(UtMemoryMappedFile *self, int prot) {
  int fd = ut_file_descriptor_get_fd(ut_local_file_get_fd(self->file));

  struct stat stat_result;
  fstat(fd, &stat_result);
  self->data_length = stat_result.st_size;

  self->data = mmap(NULL, self->data_length, prot, MAP_SHARED, fd, 0);
}

static void ut_memory_mapped_file_init(UtObject *object) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  self->file = NULL;
  self->data = NULL;
  self->data_length = 0;
}

static void ut_memory_mapped_file_cleanup(UtObject *object) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  ut_object_unref(self->file);
  if (self->data != NULL) {
    munmap(self->data, self->data_length);
  }
}

static void ut_memory_mapped_file_open_read(UtObject *object) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  ut_file_open_read(self->file);
  map(self, PROT_READ);
}

static void ut_memory_mapped_file_open_write(UtObject *object, bool create) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  ut_file_open_write(self->file, create);
  map(self, PROT_READ | PROT_WRITE);
}

static void ut_memory_mapped_file_close(UtObject *object) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  ut_file_close(self->file);
}

static uint8_t ut_memory_mapped_file_get_element(UtObject *object,
                                                 size_t index) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  return self->data[index];
}

static uint8_t *ut_memory_mapped_file_take_data(UtObject *object) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  uint8_t *copy = malloc(sizeof(uint8_t) * self->data_length);
  memcpy(copy, self->data, self->data_length);
  return copy;
}

static size_t ut_memory_mapped_file_get_length(UtObject *object) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  return self->data_length;
}

static UtObject *ut_memory_mapped_file_get_element_object(UtObject *object,
                                                          size_t index) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  return ut_uint8_new(self->data[index]);
}

static UtObject *ut_memory_mapped_file_get_sublist(UtObject *object,
                                                   size_t start, size_t count) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  // FIXME: Should do this without copying.
  UtObject *copy = ut_uint8_array_new();
  ut_uint8_list_append_block(copy, self->data + start, count);
  return copy;
}

static UtObject *ut_memory_mapped_file_copy(UtObject *object) {
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  UtObject *copy = ut_uint8_array_new();
  ut_uint8_list_append_block(copy, self->data, self->data_length);
  return copy;
}

static UtFileInterface file_interface = {
    .open_read = ut_memory_mapped_file_open_read,
    .open_write = ut_memory_mapped_file_open_write,
    .close = ut_memory_mapped_file_close};

static UtUint8ListInterface uint8_list_interface = {
    .get_element = ut_memory_mapped_file_get_element,
    .take_data = ut_memory_mapped_file_take_data};

static UtListInterface list_interface = {
    .is_mutable = false,
    .get_length = ut_memory_mapped_file_get_length,
    .get_element = ut_memory_mapped_file_get_element_object,
    .get_sublist = ut_memory_mapped_file_get_sublist,
    .copy = ut_memory_mapped_file_copy};

static UtObjectInterface object_interface = {
    .type_name = "UtMemoryMappedFile",
    .init = ut_memory_mapped_file_init,
    .cleanup = ut_memory_mapped_file_cleanup,
    .interfaces = {{&ut_file_id, &file_interface},
                   {&ut_uint8_list_id, &uint8_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_memory_mapped_file_new(const char *path) {
  UtObject *object =
      ut_object_new(sizeof(UtMemoryMappedFile), &object_interface);
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  self->file = ut_local_file_new(path);
  return object;
}

uint8_t *ut_memory_mapped_file_get_data(UtObject *object) {
  assert(ut_object_is_memory_mapped_file(object));
  UtMemoryMappedFile *self = (UtMemoryMappedFile *)object;
  return self->data;
}

bool ut_object_is_memory_mapped_file(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
