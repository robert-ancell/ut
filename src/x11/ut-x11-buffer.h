#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

UtObject *ut_x11_buffer_new();

UtObject *ut_x11_buffer_new_with_data(UtObject *data);

void ut_x11_buffer_append_card8(UtObject *object, uint8_t value);

void ut_x11_buffer_append_bool(UtObject *buffer, bool value);

void ut_x11_buffer_append_padding(UtObject *object, size_t count);

void ut_x11_buffer_append_align_padding(UtObject *object, size_t alignment);

void ut_x11_buffer_append_card16(UtObject *object, uint16_t value);

void ut_x11_buffer_append_int16(UtObject *object, int16_t value);

void ut_x11_buffer_append_card32(UtObject *object, uint32_t value);

void ut_x11_buffer_append_value_card16(UtObject *object, uint16_t value);

void ut_x11_buffer_append_value_int16(UtObject *object, int16_t value);

void ut_x11_buffer_append_string8(UtObject *object, const char *value);

uint8_t ut_x11_buffer_get_card8(UtObject *object, size_t *offset);

bool ut_x11_buffer_get_bool(UtObject *object, size_t *offset);

void ut_x11_buffer_get_padding(UtObject *object, size_t *offset, size_t count);

void ut_x11_buffer_get_align_padding(UtObject *object, size_t *offset,
                                     size_t alignment);

uint16_t ut_x11_buffer_get_card16(UtObject *object, size_t *offset);

int16_t ut_x11_buffer_get_int16(UtObject *object, size_t *offset);

uint32_t ut_x11_buffer_get_card32(UtObject *object, size_t *offset);

char *ut_x11_buffer_get_string8(UtObject *object, size_t *offset,
                                size_t length);

bool ut_object_is_x11_buffer(UtObject *object);
