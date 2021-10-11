#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_cancel_new();

void ut_cancel_activate(UtObject *object);

bool ut_cancel_is_active(UtObject *object);
