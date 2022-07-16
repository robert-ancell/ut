#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_curve25519_new();

UtObject *ut_curve25519_multiply(UtObject *object, UtObject *k, UtObject *u);

bool ut_object_is_curve25519(UtObject *object);