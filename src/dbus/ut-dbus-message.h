#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef enum {
  UT_DBUS_MESSAGE_TYPE_METHOD_CALL = 1,
  UT_DBUS_MESSAGE_TYPE_METHOD_RETURN = 2,
  UT_DBUS_MESSAGE_TYPE_ERROR = 3,
  UT_DBUS_MESSAGE_TYPE_SIGNAL = 4
} UtDBusMessageType;

typedef enum {
  UT_DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED = 0x01,
  UT_DBUS_MESSAGE_FLAG_NO_AUTO_START = 0x02,
  UT_DBUS_MESSAGE_FLAG_ALLOW_INTERACTIVE_AUTHORIZATION = 0x04
} UtDBusMessageFlag;

UtObject *ut_dbus_message_new(uint8_t type);

UtObject *ut_dbus_message_new_method_call(const char *destination,
                                          const char *path,
                                          const char *interface,
                                          const char *name, UtObject *args);

UtObject *ut_dbus_message_new_method_return(uint32_t reply_serial,
                                            UtObject *args);

UtObject *ut_dbus_message_new_error(const char *error_name, UtObject *values);

UtObject *ut_dbus_message_new_signal(const char *path, const char *interface,
                                     const char *name, UtObject *out_args);

UtDBusMessageType ut_dbus_message_get_type(UtObject *object);

void ut_dbus_message_set_flags(UtObject *object, uint8_t flags);

uint8_t ut_dbus_message_get_flags(UtObject *object);

void ut_dbus_message_set_serial(UtObject *object, uint32_t serial);

uint32_t ut_dbus_message_get_serial(UtObject *object);

void ut_dbus_message_set_destination(UtObject *object, const char *destination);

const char *ut_dbus_message_get_destination(UtObject *object);

void ut_dbus_message_set_sender(UtObject *object, const char *sender);

const char *ut_dbus_message_get_sender(UtObject *object);

void ut_dbus_message_set_path(UtObject *object, const char *path);

const char *ut_dbus_message_get_path(UtObject *object);

void ut_dbus_message_set_interface(UtObject *object, const char *interface);

const char *ut_dbus_message_get_interface(UtObject *object);

void ut_dbus_message_set_member(UtObject *object, const char *member);

const char *ut_dbus_message_get_member(UtObject *object);

void ut_dbus_message_set_error_name(UtObject *object, const char *error_name);

const char *ut_dbus_message_get_error_name(UtObject *object);

void ut_dbus_message_set_reply_serial(UtObject *object, uint32_t reply_serial);

bool ut_dbus_message_has_reply_serial(UtObject *object);

uint32_t ut_dbus_message_get_reply_serial(UtObject *object);

void ut_dbus_message_set_args(UtObject *object, UtObject *args);

UtObject *ut_dbus_message_get_args(UtObject *object);

UtObject *ut_dbus_message_encode(UtObject *object);

bool ut_object_is_dbus_message(UtObject *object);
