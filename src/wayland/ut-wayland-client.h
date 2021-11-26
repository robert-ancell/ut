#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtWaylandClientConnectCallback)(void *user_data,
                                               UtObject *error);
typedef void (*UtWaylandClientSyncCallback)(void *user_data);

UtObject *ut_wayland_client_new();

void ut_wayland_client_connect(UtObject *object,
                               UtWaylandClientConnectCallback callback,
                               void *user_data, UtObject *cancel);

void ut_wayland_client_display_sync(UtObject *object,
                                    UtWaylandClientSyncCallback callback,
                                    void *user_data, UtObject *cancel);

uint32_t ut_wayland_client_compositor_create_surface(UtObject *object);

uint32_t ut_wayland_client_shm_create_pool(UtObject *object, int fd,
                                           size_t size);

uint32_t ut_wayland_client_shell_get_shell_surface(UtObject *object,
                                                   uint32_t surface);

void ut_wayland_client_shell_surface_move(UtObject *object,
                                          uint32_t shell_surface, uint32_t seat,
                                          uint32_t serial);

void ut_wayland_client_shell_surface_resize(UtObject *object,
                                            uint32_t shell_surface,
                                            uint32_t seat, uint32_t serial,
                                            uint32_t edges);

void ut_wayland_client_shell_surface_set_toplevel(UtObject *object,
                                                  uint32_t shell_surface);

void ut_wayland_client_shell_surface_set_transient(UtObject *object,
                                                   uint32_t shell_surface,
                                                   uint32_t parent, int32_t x,
                                                   int32_t y, uint32_t flags);

void ut_wayland_client_shell_surface_set_fullscreen(UtObject *object,
                                                    uint32_t shell_surface,
                                                    uint32_t method,
                                                    uint32_t framerate,
                                                    uint32_t output);

void ut_wayland_client_shell_surface_set_popup(UtObject *object,
                                               uint32_t shell_surface,
                                               uint32_t seat, uint32_t serial,
                                               uint32_t parent, int32_t x,
                                               int32_t y, uint32_t flags);

void ut_wayland_client_shell_surface_set_maximized(UtObject *object,
                                                   uint32_t shell_surface,
                                                   uint32_t output);

void ut_wayland_client_shell_surface_set_title(UtObject *object,
                                               uint32_t shell_surface,
                                               const char *title);

void ut_wayland_client_shell_surface_set_class(UtObject *object,
                                               uint32_t shell_surface,
                                               const char *class_);

bool ut_object_is_wayland_client(UtObject *object);
