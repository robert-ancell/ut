#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

UtObject *ut_x11_buffer_new();

UtObject *ut_x11_buffer_new_from_data(UtObject *data);

UtObject *ut_x11_buffer_get_data(UtObject *object);

UtObject *ut_x11_buffer_get_fds(UtObject *object);

void ut_x11_buffer_append_card8(UtObject *object, uint8_t value);

void ut_x11_buffer_append_bool(UtObject *object, bool value);

void ut_x11_buffer_append_padding(UtObject *object, size_t count);

void ut_x11_buffer_append_align_padding(UtObject *object, size_t alignment);

void ut_x11_buffer_append_card16(UtObject *object, uint16_t value);

void ut_x11_buffer_append_int16(UtObject *object, int16_t value);

void ut_x11_buffer_append_card32(UtObject *object, uint32_t value);

void ut_x11_buffer_append_value_card16(UtObject *object, uint16_t value);

void ut_x11_buffer_append_value_int16(UtObject *object, int16_t value);

void ut_x11_buffer_append_block(UtObject *object, const uint8_t *data,
                                size_t data_length);

void ut_x11_buffer_append_string8(UtObject *object, const char *value);

void ut_x11_buffer_append_fd(UtObject *object, UtObject *fd);

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

size_t ut_x11_buffer_get_fd_count(UtObject *object);

UtObject *ut_x11_buffer_take_fd(UtObject *object);

bool ut_object_is_x11_buffer(UtObject *object);
