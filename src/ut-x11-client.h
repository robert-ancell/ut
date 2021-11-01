#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtX11ClientConnectCallback)(void *user_data, UtObject *error);
typedef void (*UtX11InternAtomCallback)(void *user_data, uint32_t atom);
typedef void (*UtX11GetAtomNameCallback)(void *user_data, const char *name);

UtObject *ut_x11_client_new();

void ut_x11_client_connect(UtObject *object,
                           UtX11ClientConnectCallback callback, void *user_data,
                           UtObject *cancel);

uint32_t ut_x11_client_create_window(UtObject *object, int16_t x, int16_t y,
                                     uint16_t width, uint16_t height);

void ut_x11_client_change_window_attributes(UtObject *object, uint32_t window);

void ut_x11_client_get_window_attributes(UtObject *object, uint32_t window);

void ut_x11_client_destroy_window(UtObject *object, uint32_t window);

void ut_x11_client_reparent_window(UtObject *object, uint32_t window,
                                   uint32_t parent, int16_t x, int16_t y);

void ut_x11_client_map_window(UtObject *object, uint32_t window);

void ut_x11_client_unmap_window(UtObject *object, uint32_t window);

void ut_x11_client_configure_window(UtObject *object, uint32_t window);

void ut_x11_client_intern_atom(UtObject *object, const char *name,
                               bool only_if_exists,
                               UtX11InternAtomCallback callback,
                               void *user_data, UtObject *cancel);

void ut_x11_client_get_atom_name(UtObject *object, uint32_t atom,
                                 UtX11GetAtomNameCallback callback,
                                 void *user_data, UtObject *cancel);

void ut_x11_client_create_pixmap(UtObject *object);

void ut_x11_client_free_pixmap(UtObject *object, uint32_t pixmap);

void ut_x11_client_query_extension(UtObject *object, const char *name);

void ut_x11_client_list_extensions(UtObject *object);

bool ut_object_is_x11_client(UtObject *object);
