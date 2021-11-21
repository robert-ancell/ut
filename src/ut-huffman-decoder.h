#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_huffman_decoder_new(UtObject *symbol_widths);

size_t ut_huffman_decoder_get_min_code_width(UtObject *object);

size_t ut_huffman_decoder_get_max_code_width(UtObject *object);

bool ut_huffman_decoder_get_symbol(UtObject *object, uint16_t code,
                                   size_t code_width, uint16_t *symbol);

bool ut_object_is_huffman_decoder(UtObject *object);
