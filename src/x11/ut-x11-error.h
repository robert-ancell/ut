#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
} UtX11ErrorInterface;

extern int ut_x11_error_id;

bool ut_object_implements_x11_error(UtObject *object);
