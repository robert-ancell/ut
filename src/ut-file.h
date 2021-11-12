#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  void (*open_read)(UtObject *object);
  void (*open_write)(UtObject *object, bool create);
  void (*close)(UtObject *object);
} UtFileInterface;

extern int ut_file_id;

void ut_file_open_read(UtObject *object);

void ut_file_open_write(UtObject *object, bool create);

void ut_file_close(UtObject *object);

bool ut_object_implements_file(UtObject *object);
