#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-dbus-auth-client.h"
#include "ut-general-error.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-output-stream.h"
#include "ut-string.h"
#include "ut-uint8-list.h"

typedef enum {
  AUTH_STATE_IDLE,
  AUTH_STATE_PROBE_MECHANISMS,
  AUTH_STATE_AUTH_EXTERNAL,
  AUTH_STATE_DONE
} AuthState;

typedef struct {
  UtObject object;
  UtObject *input_stream;
  UtObject *output_stream;
  UtObject *read_cancel;
  AuthState state;
  char *guid;
  UtObject *error;
  UtAuthCompleteCallback complete_callback;
  void *complete_user_data;
  UtObject *complete_cancel;
} UtDBusAuthClient;

static void send_auth_message(UtDBusAuthClient *self, const char *message) {
  UtObjectRef text = ut_string_new(message);
  ut_string_append(text, "\r\n");
  UtObjectRef utf8 = ut_string_get_utf8(text);
  ut_output_stream_write(self->output_stream, utf8);
}

static void send_auth_start(UtDBusAuthClient *self) {
  send_auth_message(self, "AUTH");
}

static void send_auth_external(UtDBusAuthClient *self) {
  UtObjectRef message = ut_string_new("AUTH EXTERNAL ");
  ut_cstring_ref auth_id = ut_cstring_new_printf("%d", (int)getuid());
  for (char *c = auth_id; *c != '\0'; c++) {
    ut_cstring_ref hex = ut_cstring_new_printf("%02x", *c);
    ut_string_append(message, hex);
  }
  send_auth_message(self, ut_string_get_text(message));
}

static void send_auth_end(UtDBusAuthClient *self) {
  send_auth_message(self, "BEGIN");
}

static void done(UtDBusAuthClient *self) {
  self->state = AUTH_STATE_DONE;
  ut_cancel_activate(self->read_cancel);

  if (!ut_cancel_is_active(self->complete_cancel)) {
    self->complete_callback(self->complete_user_data, self->guid,
                            ut_object_ref(self->error));
  }
}

static void process_line(UtDBusAuthClient *self, const char *line) {
  char *command_end = strchr(line, ' ');
  ut_cstring_ref command = NULL;
  ut_cstring_ref args = NULL;
  if (command_end == NULL) {
    command = strdup(line);
    args = strdup("");
  } else {
    command = strndup(line, command_end - line);
    args = strdup(command_end + 1);
  }

  if (strcmp(command, "REJECTED") == 0) {
    if (self->state == AUTH_STATE_PROBE_MECHANISMS) {
      self->state = AUTH_STATE_AUTH_EXTERNAL;
      send_auth_external(self);
    } else {
      self->error = ut_general_error_new("No supported auth mechanism");
      done(self);
    }
  } else if (strcmp(command, "OK") == 0) {
    assert(self->state == AUTH_STATE_AUTH_EXTERNAL);
    free(self->guid);
    self->guid = strdup(args);
    self->state = AUTH_STATE_DONE;
    ut_cancel_activate(self->read_cancel);
    send_auth_end(self);
    done(self);
  } else if (strcmp(command, "ERROR") == 0) {
    self->error = ut_general_error_new("Error during authentication: %s", args);
    done(self);
  } else {
    assert(false);
  }
}

static char *read_line(UtObject *data, size_t *offset) {
  size_t length = ut_list_get_length(data);

  for (size_t end = *offset; end < length - 1; end++) {
    if (ut_uint8_list_get_element(data, end) == '\r' &&
        ut_uint8_list_get_element(data, end + 1) == '\n') {
      char *line = malloc(sizeof(char) * (end + 1));
      for (size_t i = 0; i < end; i++) {
        line[i] = ut_uint8_list_get_element(data, *offset + i);
      }
      line[end] = '\0';
      *offset = end + 2;
      return line;
    }
  }

  return NULL;
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtDBusAuthClient *self = user_data;

  size_t offset = 0;
  while (self->state != AUTH_STATE_DONE) {
    ut_cstring_ref line = read_line(data, &offset);
    if (line == NULL) {
      break;
    }

    process_line(self, line);
  }

  return offset;
}

static void ut_dbus_auth_client_init(UtObject *object) {
  UtDBusAuthClient *self = (UtDBusAuthClient *)object;
  self->input_stream = NULL;
  self->output_stream = NULL;
  self->read_cancel = ut_cancel_new();
  self->state = AUTH_STATE_IDLE;
  self->guid = NULL;
  self->error = NULL;
  self->complete_callback = NULL;
  self->complete_user_data = NULL;
  self->complete_cancel = NULL;
}

static void ut_dbus_auth_client_cleanup(UtObject *object) {
  UtDBusAuthClient *self = (UtDBusAuthClient *)object;
  ut_object_unref(self->input_stream);
  ut_object_unref(self->output_stream);
  ut_object_unref(self->read_cancel);
  free(self->guid);
  ut_object_unref(self->error);
  ut_object_unref(self->complete_cancel);
}

static UtObjectInterface object_interface = {.type_name = "UtDBusAuthClient",
                                             .init = ut_dbus_auth_client_init,
                                             .cleanup =
                                                 ut_dbus_auth_client_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_dbus_auth_client_new(UtObject *input_stream,
                                  UtObject *output_stream) {
  UtObject *object = ut_object_new(sizeof(UtDBusAuthClient), &object_interface);
  UtDBusAuthClient *self = (UtDBusAuthClient *)object;
  self->input_stream = ut_object_ref(input_stream);
  self->output_stream = ut_object_ref(output_stream);
  return object;
}

void ut_dbus_auth_client_run(UtObject *object, UtAuthCompleteCallback callback,
                             void *user_data, UtObject *cancel) {
  assert(ut_object_is_dbus_auth_client(object));
  UtDBusAuthClient *self = (UtDBusAuthClient *)object;

  assert(self->state == AUTH_STATE_IDLE);
  self->state = AUTH_STATE_PROBE_MECHANISMS;

  assert(callback != NULL);
  self->complete_callback = callback;
  self->complete_user_data = user_data;
  self->complete_cancel = ut_object_ref(cancel);

  ut_input_stream_read(self->input_stream, read_cb, self, self->read_cancel);

  // Send empty byte, which was used for sending credentials (no longer
  // required).
  UtObjectRef start = ut_uint8_list_new_with_data(1, 0);
  ut_output_stream_write(self->output_stream, start);

  send_auth_start(self);
}

bool ut_object_is_dbus_auth_client(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
