#include "ut-object.h"

#pragma once

char *ut_base64_encode(UtObject *object);

UtObject *ut_base64_decode(const char *text);
