#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtX11ClientEventCallback)(void *user_data, UtObject *event);
typedef void (*UtX11ClientErrorCallback)(void *user_data, UtObject *error);
typedef void (*UtX11ClientConnectCallback)(void *user_data, UtObject *error);
typedef void (*UtX11InternAtomCallback)(void *user_data, uint32_t atom,
                                        UtObject *error);
typedef void (*UtX11GetAtomNameCallback)(void *user_data, const char *name,
                                         UtObject *error);
typedef void (*UtX11GetPropertyCallback)(void *user_data, uint32_t type,
                                         UtObject *value, uint32_t bytes_after,
                                         UtObject *error);
typedef void (*UtX11ListPropertiesCallback)(void *user_data, UtObject *atoms,
                                            UtObject *error);
typedef void (*UtX11ListExtensionsCallback)(void *user_data, UtObject *names,
                                            UtObject *error);

typedef enum {
  UT_X11_IMAGE_FORMAT_BITMAP,
  UT_X11_IMAGE_FORMAT_XY_PIXMAP,
  UT_X11_IMAGE_FORMAT_Z_PIXMAP
} UtX11ImageFormat;

UtObject *ut_x11_client_new(UtX11ClientEventCallback event_callback,
                            UtX11ClientErrorCallback error_callback,
                            void *user_data, UtObject *cancel);

void ut_x11_client_connect(UtObject *object,
                           UtX11ClientConnectCallback callback, void *user_data,
                           UtObject *cancel);

uint32_t ut_x11_client_create_window(UtObject *object, int16_t x, int16_t y,
                                     uint16_t width, uint16_t height);

void ut_x11_client_destroy_window(UtObject *object, uint32_t window);

void ut_x11_client_map_window(UtObject *object, uint32_t window);

void ut_x11_client_unmap_window(UtObject *object, uint32_t window);

void ut_x11_client_configure_window(UtObject *object, uint32_t window,
                                    int16_t x, int16_t y, uint16_t width,
                                    uint16_t height);

void ut_x11_client_intern_atom(UtObject *object, const char *name,
                               bool only_if_exists,
                               UtX11InternAtomCallback callback,
                               void *user_data, UtObject *cancel);

void ut_x11_client_get_atom_name(UtObject *object, uint32_t atom,
                                 UtX11GetAtomNameCallback callback,
                                 void *user_data, UtObject *cancel);

void ut_x11_client_change_property(UtObject *object, uint32_t window,
                                   uint32_t property, uint32_t type);

void ut_x11_client_delete_property(UtObject *object, uint32_t window,
                                   uint32_t property);

void ut_x11_client_get_property(UtObject *object, uint32_t window,
                                uint32_t property,
                                UtX11GetPropertyCallback callback,
                                void *user_data, UtObject *cancel);

void ut_x11_client_get_property_full(UtObject *object, uint32_t window,
                                     uint32_t property, uint32_t type,
                                     uint32_t long_offset, uint32_t long_length,
                                     bool delete,
                                     UtX11GetPropertyCallback callback,
                                     void *user_data, UtObject *cancel);

void ut_x11_client_list_properties(UtObject *object, uint32_t window,
                                   UtX11ListPropertiesCallback callback,
                                   void *user_data, UtObject *cancel);

uint32_t ut_x11_client_create_pixmap(UtObject *object, uint32_t drawable,
                                     uint16_t width, uint16_t height,
                                     uint8_t depth);

void ut_x11_client_free_pixmap(UtObject *object, uint32_t pixmap);

uint32_t ut_x11_client_create_gc(UtObject *object, uint32_t drawable);

void ut_x11_client_free_gc(UtObject *object, uint32_t gc);

void ut_x11_client_clear_area(UtObject *object, uint32_t window, int16_t x,
                              int16_t y, uint16_t width, uint16_t height,
                              bool exposures);

void ut_x11_client_copy_area(UtObject *object, uint32_t src_drawable,
                             uint32_t dst_drawable, uint32_t gc, int16_t src_x,
                             int16_t src_y, int16_t dst_x, int16_t dst_y,
                             uint16_t width, uint16_t height);

void ut_x11_client_put_image(UtObject *object, uint32_t drawable, uint32_t gc,
                             UtX11ImageFormat format, uint16_t width,
                             uint16_t height, uint8_t depth, int16_t dst_x,
                             int16_t dst_y, uint8_t *data, size_t data_length);

void ut_x11_client_list_extensions(UtObject *object,
                                   UtX11ListExtensionsCallback callback,
                                   void *user_data, UtObject *cancel);

void ut_x11_client_bell(UtObject *object);

void ut_x11_client_kill_client(UtObject *object, uint32_t resource);

UtObject *ut_x11_client_get_mit_shm_extension(UtObject *object);

UtObject *ut_x11_client_get_present_extension(UtObject *object);

bool ut_object_is_x11_client(UtObject *object);
