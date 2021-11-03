#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
} UtX11EventInterface;

extern int ut_x11_event_id;

bool ut_object_implements_x11_event(UtObject *object);
