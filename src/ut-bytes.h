#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_bytes_new(const uint8_t *data, size_t length);

const uint8_t *ut_bytes_get_data(UtObject *object);
