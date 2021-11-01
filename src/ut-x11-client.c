#include <assert.h>
#include <string.h>

#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-end-of-stream.h"
#include "ut-general-error.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-output-stream.h"
#include "ut-string-list.h"
#include "ut-string.h"
#include "ut-uint16-list.h"
#include "ut-uint32-list.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-unix-domain-socket-client.h"
#include "ut-x11-client.h"

typedef struct _UtX11Client UtX11Client;
typedef struct _Request Request;

typedef void (*DecodeReplyFunction)(UtX11Client *self, Request *request,
                                    uint8_t data0, UtObject *data,
                                    size_t *offset);

typedef void (*RequestCallback)(void *user_data);

typedef enum {
  WINDOW_CLASS_INHERIT_FROM_PARENT = 0,
  WINDOW_CLASS_INPUT_OUTPUT = 1,
  WINDOW_CLASS_INPUT_ONLY = 2
} UtX11WindowClass;

typedef enum {
  VALUE_MASK_BACKGROUND_PIXMAP = 0x00000001,
  VALUE_MASK_BACKGROUND_PIXEL = 0x00000002,
  VALUE_MASK_BORDER_PIXMAP = 0x00000004,
  VALUE_MASK_BORDER_PIXEL = 0x00000008,
  VALUE_MASK_BIT_GRAVITY = 0x00000010,
  VALUE_MASK_WIN_GRAVITY = 0x00000020,
  VALUE_MASK_BACKING_STORE = 0x00000040,
  VALUE_MASK_BACKING_PLANES = 0x00000080,
  VALUE_MASK_BACKING_PIXEL = 0x00000100,
  VALUE_MASK_OVERRIDE_REDIRECT = 0x00000200,
  VALUE_MASK_SAVE_UNDER = 0x00000400,
  VALUE_MASK_EVENT_MASK = 0x00000800,
  VALUE_MASK_DO_NOT_PROPAGATE_MASK = 0x00001000,
  VALUE_MASK_COLORMAP = 0x00002000,
  VALUE_MASK_CURSOR = 0x00004000
} UtX11ValueMask;

typedef enum {
  EVENT_KEY_PRESS = 0x00000001,
  EVENT_KEY_RELEASE = 0x00000002,
  EVENT_BUTTON_PRESS = 0x00000004,
  EVENT_BUTTON_RELEASE = 0x00000008,
  EVENT_ENTER_WINDOW = 0x00000010,
  EVENT_LEAVE_WINDOW = 0x00000020,
  EVENT_POINTER_MOTION = 0x00000040,
  EVENT_POINTER_MOTION_HINT = 0x00000080,
  EVENT_BUTTON1_MOTION = 0x00000100,
  EVENT_BUTTON2_MOTION = 0x00000200,
  EVENT_BUTTON3_MOTION = 0x00000400,
  EVENT_BUTTON4_MOTION = 0x00000800,
  EVENT_BUTTON5_MOTION = 0x00001000,
  EVENT_BUTTON_MOTION = 0x00002000,
  EVENT_KEYMAP_STATE = 0x00004000,
  EVENT_EXPOSURE = 0x00008000,
  EVENT_VISIBILITY_CHANGE = 0x00010000,
  EVENT_STRUCTURE_NOTIFY = 0x00020000,
  EVENT_RESIZE_REDIRECT = 0x00040000,
  EVENT_SUBSTRUCTURE_NOTIFY = 0x00080000,
  EVENT_SUBSTRUCTURE_REDIRECT = 0x00100000,
  EVENT_FOCUS_CHANGE = 0x00200000,
  EVENT_PROPERTY_CHANGE = 0x00400000,
  EVENT_COLORMAP_CHANGE = 0x00800000,
  EVENT_OWNER_GRAB_BUTTON = 0x01000000
} UtX11Event;

typedef struct {
  uint8_t depth;
  uint8_t bits_per_pixel;
  uint8_t scanline_pad;
} X11PixmapFormat;

typedef struct {
  uint32_t id;
  uint8_t depth;
  uint8_t class;
  uint8_t bits_per_rgb_value;
  uint16_t colormap_entries;
  uint32_t red_mask;
  uint32_t green_mask;
  uint32_t blue_mask;
} X11Visual;

typedef struct {
  uint32_t root;
  uint32_t default_colormap;
  uint32_t white_pixel;
  uint32_t black_pixel;
  uint32_t current_input_masks;
  uint16_t width_in_pixels;
  uint16_t height_in_pixels;
  uint16_t width_in_millimeters;
  uint16_t height_in_millimeters;
  X11Visual *root_visual;
  X11Visual **visuals;
  size_t visuals_length;
} X11Screen;

struct _Request {
  uint16_t sequence_number;
  DecodeReplyFunction decode_reply_function;
  RequestCallback callback;
  void *user_data;
  UtObject *cancel;
  Request *next;
};

struct _UtX11Client {
  UtObject object;
  UtObject *socket;
  UtObject *read_cancel;

  UtX11ClientConnectCallback connect_callback;
  void *connect_user_data;
  UtObject *connect_cancel;

  bool connected;
  char *vendor;
  uint32_t release_number;
  uint32_t resource_id_base;
  uint32_t resource_id_mask;
  uint16_t maximum_request_length;
  X11PixmapFormat **pixmap_formats;
  size_t pixmap_formats_length;
  X11Screen **screens;
  size_t screens_length;

  uint32_t next_resource_id;
  uint16_t sequence_number;
  Request *requests;
};

#include <stdio.h> // FIXME: Remove

static uint32_t create_resource_id(UtX11Client *self) {
  return self->resource_id_base | (self->next_resource_id++);
}

static void write_card8(UtObject *buffer, uint8_t value) {
  ut_uint8_array_append(buffer, value);
}

static void write_padding(UtObject *buffer, size_t count) {
  for (size_t i = 0; i < count; i++) {
    write_card8(buffer, 0);
  }
}

static void write_align_padding(UtObject *buffer, size_t alignment) {
  size_t buffer_length = ut_list_get_length(buffer);
  size_t extra = buffer_length % alignment;
  if (extra != 0) {
    write_padding(buffer, alignment - extra);
  }
}

static void write_card16(UtObject *buffer, uint16_t value) {
  ut_uint8_array_append(buffer, value & 0xff);
  ut_uint8_array_append(buffer, value >> 8);
}

static void write_int16(UtObject *buffer, int16_t value) {
  write_card16(buffer, (uint16_t)value);
}

static void write_card32(UtObject *buffer, uint32_t value) {
  ut_uint8_array_append(buffer, value & 0xff);
  ut_uint8_array_append(buffer, (value >> 8) & 0xff);
  ut_uint8_array_append(buffer, (value >> 16) & 0xff);
  ut_uint8_array_append(buffer, value >> 24);
}

static void write_value_card16(UtObject *buffer, uint16_t value) {
  write_card32(buffer, value);
}

static void write_value_int16(UtObject *buffer, int16_t value) {
  write_value_card16(buffer, (uint16_t)value);
}

static void write_string8(UtObject *buffer, const char *value) {
  for (const char *c = value; *c != '\0'; c++) {
    write_card8(buffer, *c);
  }
}

static void send_request_with_reply(UtX11Client *self, uint8_t opcode,
                                    uint8_t data0, UtObject *data,
                                    DecodeReplyFunction decode_reply_function,
                                    RequestCallback callback, void *user_data,
                                    UtObject *cancel) {
  size_t data_length = ut_list_get_length(data);
  assert(data_length % 4 == 0);

  UtObjectRef request = ut_uint8_array_new();
  write_card8(request, opcode);
  write_card8(request, data0);
  write_card16(request, 1 + data_length / 4);
  ut_uint8_array_append_block(request, ut_uint8_array_get_data(data),
                              data_length);

  self->sequence_number++;

  if (decode_reply_function != NULL) {
    Request *request = malloc(sizeof(Request));
    request->sequence_number = self->sequence_number;
    request->decode_reply_function = decode_reply_function;
    request->callback = callback;
    request->user_data = user_data;
    request->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
    request->next = self->requests;
    self->requests = request;
  }

  ut_output_stream_write(self->socket, request);
}

static void send_request(UtX11Client *self, uint8_t opcode, uint8_t data0,
                         UtObject *data) {
  send_request_with_reply(self, opcode, data0, data, NULL, NULL, NULL, NULL);
}

static uint8_t peek_card8(UtObject *buffer, size_t *offset) {
  return ut_uint8_list_get_element(buffer, *offset);
}

static uint8_t read_card8(UtObject *buffer, size_t *offset) {
  uint8_t value = peek_card8(buffer, offset);
  (*offset)++;
  return value;
}

static bool read_bool(UtObject *buffer, size_t *offset) {
  return read_card8(buffer, offset) != 0;
}

static size_t get_remaining(UtObject *buffer, size_t *offset) {
  return ut_list_get_length(buffer) - *offset;
}

static void read_padding(UtObject *buffer, size_t *offset, size_t count) {
  (*offset) += count;
}

static void read_align_padding(UtObject *buffer, size_t *offset,
                               size_t alignment) {
  while ((*offset) % alignment != 0) {
    (*offset)++;
  }
}

static uint16_t read_card16(UtObject *buffer, size_t *offset) {
  uint16_t byte1 = read_card8(buffer, offset);
  uint16_t byte2 = read_card8(buffer, offset);
  return byte1 | byte2 << 8;
}

static int16_t read_int16(UtObject *buffer, size_t *offset) {
  return (int16_t)read_card16(buffer, offset);
}

static uint32_t read_card32(UtObject *buffer, size_t *offset) {
  uint32_t byte1 = read_card8(buffer, offset);
  uint32_t byte2 = read_card8(buffer, offset);
  uint32_t byte3 = read_card8(buffer, offset);
  uint32_t byte4 = read_card8(buffer, offset);
  return byte1 | byte2 << 8 | byte3 << 16 | byte4 << 24;
}

static char *read_string8(UtObject *buffer, size_t *offset, size_t length) {
  assert(ut_list_get_length(buffer) >= *offset + length);
  char *string =
      strndup((const char *)ut_uint8_array_get_data(buffer) + *offset, length);
  (*offset) += length;
  return string;
}

static bool decode_setup_failed(UtX11Client *self, UtObject *data,
                                size_t *offset) {
  size_t o = *offset;

  size_t data_length = get_remaining(data, &o);
  if (data_length < 8) {
    return false;
  }

  assert(read_card8(data, &o) == 0);
  uint8_t reason_length = read_card8(data, &o);
  /*uint16_t protocol_major_version = */ read_card16(data, &o);
  /*uint16_t protocol_minor_version = */ read_card16(data, &o);
  uint16_t length = read_card16(data, &o);
  size_t message_length = (length + 2) * 4;
  if (data_length < message_length) {
    return false;
  }
  ut_cstring reason = read_string8(data, &o, reason_length);

  UtObjectRef error =
      ut_general_error_new("Failed to connect to X server: %s", reason);
  self->connect_callback(self->connect_user_data, error);

  *offset = o;
  return true;
}

static bool decode_setup_success(UtX11Client *self, UtObject *data,
                                 size_t *offset) {
  size_t o = *offset;

  size_t data_length = get_remaining(data, &o);
  if (data_length < 8) {
    return false;
  }

  assert(read_card8(data, &o) == 1);
  read_padding(data, &o, 1);
  /*uint16_t protocol_major_version = */ read_card16(data, &o);
  /*uint16_t protocol_minor_version = */ read_card16(data, &o);
  uint16_t length = read_card16(data, &o);
  size_t message_length = (length + 2) * 4;
  if (data_length < message_length) {
    return false;
  }
  self->release_number = read_card32(data, &o);
  self->resource_id_base = read_card32(data, &o);
  self->resource_id_mask = read_card32(data, &o);
  read_card32(data, &o); // motion_buffer_size
  uint16_t vendor_length = read_card16(data, &o);
  self->maximum_request_length = read_card16(data, &o);
  size_t screens_length = read_card8(data, &o);
  size_t pixmap_formats_length = read_card8(data, &o);
  read_card8(data, &o); // image_byte_order
  read_card8(data, &o); // bitmap_format_bit_order
  read_card8(data, &o); // bitmap_format_scanline_unit
  read_card8(data, &o); // bitmap_format_scanline_pad
  read_card8(data, &o); // min_keycode
  read_card8(data, &o); // max_keycode
  read_padding(data, &o, 4);
  self->vendor = read_string8(data, &o, vendor_length);
  read_align_padding(data, &o, 4);

  self->pixmap_formats =
      malloc(sizeof(X11PixmapFormat *) * pixmap_formats_length);
  self->pixmap_formats_length = pixmap_formats_length;
  for (size_t i = 0; i < pixmap_formats_length; i++) {
    X11PixmapFormat *format = self->pixmap_formats[i] =
        malloc(sizeof(X11PixmapFormat));
    format->depth = read_card8(data, &o);          // depth
    format->bits_per_pixel = read_card8(data, &o); // bits_per_pixel
    format->scanline_pad = read_card8(data, &o);   // scanline_pad
    read_padding(data, &o, 5);
  }

  self->screens = malloc(sizeof(X11Screen *) * screens_length);
  self->screens_length = screens_length;
  for (size_t i = 0; i < screens_length; i++) {
    X11Screen *screen = self->screens[i] = malloc(sizeof(X11Screen));

    screen->root = read_card32(data, &o);
    screen->default_colormap = read_card32(data, &o);
    screen->white_pixel = read_card32(data, &o);
    screen->black_pixel = read_card32(data, &o);
    screen->current_input_masks = read_card32(data, &o);
    screen->width_in_pixels = read_card16(data, &o);
    screen->height_in_pixels = read_card16(data, &o);
    screen->width_in_millimeters = read_card16(data, &o);
    screen->height_in_millimeters = read_card16(data, &o);
    read_card16(data, &o); // min_installed_maps
    read_card16(data, &o); // max_installed_maps
    screen->root_visual = NULL;
    uint32_t root_visual_id = read_card32(data, &o);
    read_card8(data, &o); // backing_stores
    read_card8(data, &o); // save_unders
    read_card8(data, &o); // root_depth
    screen->visuals = NULL;
    screen->visuals_length = 0;
    size_t allowed_depths_length = read_card8(data, &o);
    for (size_t j = 0; j < allowed_depths_length; j++) {
      uint8_t depth = read_card8(data, &o);
      read_padding(data, &o, 1);
      size_t visuals_length = read_card16(data, &o);
      read_padding(data, &o, 4);
      size_t first_visual = screen->visuals_length;
      screen->visuals_length += visuals_length;
      screen->visuals = realloc(screen->visuals,
                                sizeof(X11Visual *) * screen->visuals_length);
      for (size_t k = 0; k < visuals_length; k++) {
        X11Visual *visual = screen->visuals[first_visual + k] =
            malloc(sizeof(X11Visual));
        visual->id = read_card32(data, &o);
        visual->depth = depth;
        visual->class = read_card8(data, &o);
        visual->bits_per_rgb_value = read_card8(data, &o);
        visual->colormap_entries = read_card16(data, &o);
        visual->red_mask = read_card32(data, &o);
        visual->blue_mask = read_card32(data, &o);
        visual->green_mask = read_card32(data, &o);
        read_padding(data, &o, 4);

        if (visual->id == root_visual_id) {
          screen->root_visual = visual;
        }
      }
    }
  }

  self->connected = true;

  self->connect_callback(self->connect_user_data, NULL);

  *offset = o;
  return true;
}

static bool decode_setup_authenticate(UtObject *data, size_t *offset) {
  return false;
}

static bool decode_setup_message(UtX11Client *self, UtObject *data,
                                 size_t *offset) {
  uint8_t status = peek_card8(data, offset);
  if (status == 0) {
    return decode_setup_failed(self, data, offset);
  } else if (status == 1) {
    return decode_setup_success(self, data, offset);
  } else if (status == 2) {
    return decode_setup_authenticate(data, offset);
  } else {
    assert(false);
  }
}

static bool decode_error(UtX11Client *self, UtObject *data, size_t *offset) {
  if (get_remaining(data, offset) < 32) {
    return false;
  }

  assert(read_card8(data, offset) == 0);
  uint8_t code = read_card8(data, offset);
  uint16_t sequence_number = read_card16(data, offset);

  if (code == 1) {
    read_padding(data, offset, 4);
    uint16_t minor_opcode = read_card16(data, offset);
    uint8_t major_opcode = read_card8(data, offset);
    read_padding(data, offset, 21);
    printf("XServer >> RequestError opcode %d.%d seq %d\n", major_opcode,
           minor_opcode, sequence_number);
  } else if (code == 3) {
    uint32_t window = read_card32(data, offset);
    uint16_t minor_opcode = read_card16(data, offset);
    uint8_t major_opcode = read_card8(data, offset);
    read_padding(data, offset, 21);
    printf("XServer >> WindowError opcode %d.%d window %08x seq %d\n",
           major_opcode, minor_opcode, window, sequence_number);
  } else if (code == 14) {
    uint32_t resource = read_card32(data, offset);
    uint16_t minor_opcode = read_card16(data, offset);
    uint8_t major_opcode = read_card8(data, offset);
    read_padding(data, offset, 21);
    printf("XServer >> IDChoiceError opcode %d.%d resource %08x seq %d\n",
           major_opcode, minor_opcode, resource, sequence_number);
  } else if (code == 16) {
    read_padding(data, offset, 4);
    uint16_t minor_opcode = read_card16(data, offset);
    uint8_t major_opcode = read_card8(data, offset);
    read_padding(data, offset, 21);
    printf("XServer >> LengthError opcode %d.%d seq %d\n", major_opcode,
           minor_opcode, sequence_number);
  } else {
    read_padding(data, offset, 28);
    printf("XServer >> Error %d seq %d\n", code, sequence_number);
  }

  return true;
}

static void decode_intern_atom_reply(UtX11Client *self, Request *request,
                                     uint8_t data0, UtObject *data,
                                     size_t *offset) {
  uint32_t atom = read_card32(data, offset);
  read_padding(data, offset, 20);

  if (request->callback != NULL &&
      (request->cancel == NULL || !ut_cancel_is_active(request->cancel))) {
    UtX11InternAtomCallback callback =
        (UtX11InternAtomCallback)request->callback;
    callback(request->user_data, atom, NULL);
  }
}

static void decode_get_atom_name_reply(UtX11Client *self, Request *request,
                                       uint8_t data0, UtObject *data,
                                       size_t *offset) {
  uint16_t name_length = read_card16(data, offset);
  read_padding(data, offset, 22);
  ut_cstring name = read_string8(data, offset, name_length);
  read_align_padding(data, offset, 4);

  if (request->callback != NULL &&
      (request->cancel == NULL || !ut_cancel_is_active(request->cancel))) {
    UtX11GetAtomNameCallback callback =
        (UtX11GetAtomNameCallback)request->callback;
    callback(request->user_data, name, NULL);
  }
}

static void decode_get_property_reply(UtX11Client *self, Request *request,
                                      uint8_t data0, UtObject *data,
                                      size_t *offset) {
  uint8_t format = data0;
  uint32_t type = read_card32(data, offset);
  uint32_t bytes_after = read_card32(data, offset);
  size_t length = read_card32(data, offset);
  read_padding(data, offset, 12);
  UtObjectRef value = NULL;
  if (format == 0) {
  } else if (format == 8) {
    value = ut_uint8_array_new();
    for (size_t i = 0; i < length; i++) {
      ut_uint8_array_append(value, read_card8(data, offset));
    }
  } else if (format == 16) {
    value = ut_uint16_list_new();
    for (size_t i = 0; i < length; i++) {
      ut_uint16_list_append(value, read_card16(data, offset));
    }
  } else if (format == 32) {
    value = ut_uint32_list_new();
    for (size_t i = 0; i < length; i++) {
      ut_uint32_list_append(value, read_card32(data, offset));
    }
  } else {
    assert(false);
  }
  read_align_padding(data, offset, 4);

  if (request->callback != NULL &&
      (request->cancel == NULL || !ut_cancel_is_active(request->cancel))) {
    UtX11GetPropertyCallback callback =
        (UtX11GetPropertyCallback)request->callback;
    callback(request->user_data, type, value, bytes_after, NULL);
  }
}

static void decode_list_properties_reply(UtX11Client *self, Request *request,
                                         uint8_t data0, UtObject *data,
                                         size_t *offset) {
  size_t atoms_length = read_card16(data, offset);
  read_padding(data, offset, 22);
  UtObjectRef atoms = ut_uint32_list_new();
  for (size_t i = 0; i < atoms_length; i++) {
    ut_uint32_list_append(atoms, read_card32(data, offset));
  }

  if (request->callback != NULL &&
      (request->cancel == NULL || !ut_cancel_is_active(request->cancel))) {
    UtX11ListPropertiesCallback callback =
        (UtX11ListPropertiesCallback)request->callback;
    callback(request->user_data, atoms, NULL);
  }
}

static void decode_list_extensions_reply(UtX11Client *self, Request *request,
                                         uint8_t data0, UtObject *data,
                                         size_t *offset) {
  read_padding(data, offset, 24);
  size_t names_length = data0;
  UtObjectRef names = ut_string_list_new();
  for (size_t i = 0; i < names_length; i++) {
    uint8_t name_length = read_card8(data, offset);
    ut_cstring name = read_string8(data, offset, name_length);
    ut_string_list_append(names, name);
  }
  read_align_padding(data, offset, 4);

  if (request->callback != NULL &&
      (request->cancel == NULL || !ut_cancel_is_active(request->cancel))) {
    UtX11ListExtensionsCallback callback =
        (UtX11ListExtensionsCallback)request->callback;
    callback(request->user_data, names, NULL);
  }
}

static void decode_query_extension_reply(UtX11Client *self, Request *request,
                                         uint8_t data0, UtObject *data,
                                         size_t *offset) {
  uint8_t major_opcode = read_card8(data, offset);
  bool present = read_bool(data, offset);
  uint8_t first_event = read_card8(data, offset);
  uint8_t first_error = read_card8(data, offset);
  read_padding(data, offset, 20);

  if (request->callback != NULL &&
      (request->cancel == NULL || !ut_cancel_is_active(request->cancel))) {
    UtX11QueryExtensionCallback callback =
        (UtX11QueryExtensionCallback)request->callback;
    callback(request->user_data, present, major_opcode, first_event,
             first_error, NULL);
  }
}

static Request *find_request(UtX11Client *self, uint16_t sequence_number) {
  for (Request *request = self->requests; request != NULL;
       request = request->next) {
    if (request->sequence_number == sequence_number) {
      return request;
    }
  }

  return NULL;
}

static bool decode_reply(UtX11Client *self, UtObject *data, size_t *offset) {
  if (get_remaining(data, offset) < 4) {
    return false;
  }

  size_t o = *offset;
  assert(read_card8(data, &o) == 1);
  uint8_t data0 = read_card8(data, &o);
  uint16_t sequence_number = read_card16(data, &o);
  uint32_t length = read_card32(data, &o);

  size_t payload_length = 24 + length * 4;
  if (get_remaining(data, &o) < payload_length) {
    return false;
  }

  Request *request = find_request(self, sequence_number);
  if (request != NULL) {
    size_t reply_offset = o;
    request->decode_reply_function(self, request, data0, data, &reply_offset);
  } else {
    printf("XServer >> Reply seq %d\n", sequence_number);
  }
  read_padding(data, &o, payload_length);

  *offset = o;
  return true;
}

static void decode_key_press(UtObject *data, size_t *offset) {
  uint8_t keycode = read_card8(data, offset);
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // time
  read_card32(data, offset); // root
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // child
  read_int16(data, offset);  // root_x
  read_int16(data, offset);  // root_y
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  read_card16(data, offset); // state
  read_card8(data, offset);  // same_screen
  read_padding(data, offset, 1);
  printf("XServer >> KeyPress %08x %d %d,%d\n", window, keycode, x, y);
}

static void decode_key_release(UtObject *data, size_t *offset) {
  uint8_t keycode = read_card8(data, offset);
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // time
  read_card32(data, offset); // root
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // child
  read_int16(data, offset);  // root_x
  read_int16(data, offset);  // root_y
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  read_card16(data, offset); // state
  read_card8(data, offset);  // same_screen
  read_padding(data, offset, 1);
  printf("XServer >> KeyRelease %08x %d %d,%d\n", window, keycode, x, y);
}

static void decode_button_press(UtObject *data, size_t *offset) {
  uint8_t button = read_card8(data, offset);
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // time
  read_card32(data, offset); // root
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // child
  read_int16(data, offset);  // root_x
  read_int16(data, offset);  // root_y
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  read_card16(data, offset); // state
  read_card8(data, offset);  // same_screen
  read_padding(data, offset, 1);
  printf("XServer >> ButtonPress %08x %d %d,%d\n", window, button, x, y);
}

static void decode_button_release(UtObject *data, size_t *offset) {
  uint8_t button = read_card8(data, offset);
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // time
  read_card32(data, offset); // root
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // child
  read_int16(data, offset);  // root_x
  read_int16(data, offset);  // root_y
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  read_card16(data, offset); // state
  read_card8(data, offset);  // same_screen
  read_padding(data, offset, 1);
  printf("XServer >> ButtonRelease %08x %d %d,%d\n", window, button, x, y);
}

static void decode_motion_notify(UtObject *data, size_t *offset) {
  read_card8(data, offset);  // detail
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // time
  read_card32(data, offset); // root
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // child
  read_int16(data, offset);  // root_x
  read_int16(data, offset);  // root_y
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  read_card16(data, offset); // state
  read_card8(data, offset);  // same_screen
  read_padding(data, offset, 1);
  printf("XServer >> MotionNotify %08x %d,%d\n", window, x, y);
}

static void decode_enter_notify(UtObject *data, size_t *offset) {
  read_card8(data, offset);  // detail
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // time
  read_card32(data, offset); // root
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // child
  read_int16(data, offset);  // root_x
  read_int16(data, offset);  // root_y
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  read_card16(data, offset); // state
  read_card8(data, offset);  // mode
  read_card8(data, offset);  // same_screen, focus
  printf("XServer >> EnterNotify %08x %d,%d\n", window, x, y);
}

static void decode_leave_notify(UtObject *data, size_t *offset) {
  read_card8(data, offset);  // detail
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // time
  read_card32(data, offset); // root
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // child
  read_int16(data, offset);  // root_x
  read_int16(data, offset);  // root_y
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  read_card16(data, offset); // state
  read_card8(data, offset);  // mode
  read_card8(data, offset);  // same_screen, focus
  printf("XServer >> LeaveNotify %08x %d,%d\n", window, x, y);
}

static void decode_expose(UtObject *data, size_t *offset) {
  read_padding(data, offset, 1);
  read_card16(data, offset); // sequence_number
  uint32_t window = read_card32(data, offset);
  uint16_t x = read_card16(data, offset);
  uint16_t y = read_card16(data, offset);
  uint16_t width = read_card16(data, offset);
  uint16_t height = read_card16(data, offset);
  read_card16(data, offset); // count
  read_padding(data, offset, 14);
  printf("XServer >> Expose %08x %d,%d %dx%d\n", window, x, y, width, height);
}

static void decode_configure_notify(UtObject *data, size_t *offset) {
  read_padding(data, offset, 1);
  read_card16(data, offset); // sequence_number
  read_card32(data, offset); // event
  uint32_t window = read_card32(data, offset);
  read_card32(data, offset); // above_sibling
  int16_t x = read_int16(data, offset);
  int16_t y = read_int16(data, offset);
  uint16_t width = read_card16(data, offset);
  uint16_t height = read_card16(data, offset);
  read_card16(data, offset); // border_width
  read_card8(data, offset);  // override_redirect
  read_padding(data, offset, 5);
  printf("XServer >> ConfigureNotify %08x %d,%d %dx%d\n", window, x, y, width,
         height);
}

static void decode_property_notify(UtObject *data, size_t *offset) {
  read_padding(data, offset, 1);
  read_card16(data, offset); // sequence_number
  uint32_t window = read_card32(data, offset);
  uint32_t atom = read_card32(data, offset);
  read_card32(data, offset); // time
  read_card8(data, offset);  // state
  read_padding(data, offset, 15);
  printf("XServer >> PropertyNotify %08x %08x\n", window, atom);
}

static bool decode_event(UtX11Client *self, UtObject *data, size_t *offset) {
  if (get_remaining(data, offset) < 32) {
    return false;
  }

  uint8_t code = read_card8(data, offset);
  if (code == 2) {
    decode_key_press(data, offset);
  } else if (code == 3) {
    decode_key_release(data, offset);
  } else if (code == 4) {
    decode_button_press(data, offset);
  } else if (code == 5) {
    decode_button_release(data, offset);
  } else if (code == 6) {
    decode_motion_notify(data, offset);
  } else if (code == 7) {
    decode_enter_notify(data, offset);
  } else if (code == 8) {
    decode_leave_notify(data, offset);
  } else if (code == 12) {
    decode_expose(data, offset);
  } else if (code == 22) {
    decode_configure_notify(data, offset);
  } else if (code == 28) {
    decode_property_notify(data, offset);
  } else {
    read_padding(data, offset, 31);
    printf("XServer >> Event %d\n", code);
  }

  return true;
}

static bool decode_message(UtX11Client *self, UtObject *data, size_t *offset) {
  uint8_t code = peek_card8(data, offset);
  if (code == 0) {
    return decode_error(self, data, offset);
  } else if (code == 1) {
    return decode_reply(self, data, offset);
  } else {
    return decode_event(self, data, offset);
  }
}

static size_t read_cb(void *user_data, UtObject *data) {
  UtX11Client *self = user_data;

  if (ut_object_is_end_of_stream(data)) {
    ut_cancel_activate(self->read_cancel);
    return 0;
  }

  size_t offset = 0;
  while (!self->connected) {
    if (!decode_setup_message(self, data, &offset)) {
      return offset;
    }
  }

  while (get_remaining(data, &offset) > 0) {
    if (!decode_message(self, data, &offset)) {
      return offset;
    }
  }

  return offset;
}

static void ut_x11_client_init(UtObject *object) {
  UtX11Client *self = (UtX11Client *)object;
  self->socket = NULL;
  self->read_cancel = ut_cancel_new();
  self->connect_callback = NULL;
  self->connect_user_data = NULL;
  self->connect_cancel = NULL;
  self->connected = false;
  self->vendor = NULL;
  self->release_number = 0;
  self->resource_id_base = 0;
  self->resource_id_mask = 0;
  self->maximum_request_length = 0;
  self->pixmap_formats = NULL;
  self->pixmap_formats_length = 0;
  self->screens = NULL;
  self->screens_length = 0;
  self->next_resource_id = 0;
  self->sequence_number = 0;
  self->requests = NULL;
}

static void ut_x11_client_cleanup(UtObject *object) {
  UtX11Client *self = (UtX11Client *)object;
  if (self->socket != NULL) {
    ut_object_unref(self->socket);
  }
  ut_object_unref(self->read_cancel);
  if (self->connect_cancel != NULL) {
    ut_object_unref(self->connect_cancel);
  }
  free(self->vendor);
  for (size_t i = 0; i < self->pixmap_formats_length; i++) {
    free(self->pixmap_formats[i]);
  }
  free(self->pixmap_formats);
  for (size_t i = 0; i < self->screens_length; i++) {
    X11Screen *screen = self->screens[i];
    for (size_t j = 0; j < screen->visuals_length; j++) {
      free(screen->visuals[j]);
    }
    free(screen);
  }
  free(self->screens);
  Request *next_request;
  for (Request *request = self->requests; request != NULL;
       request = next_request) {
    next_request = request->next;
    if (request->cancel != NULL) {
      ut_object_unref(request->cancel);
    }
    free(request);
  }
}

static UtObjectInterface object_interface = {.type_name = "UtX11Client",
                                             .init = ut_x11_client_init,
                                             .cleanup = ut_x11_client_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_x11_client_new(const char *path) {
  UtObject *object = ut_object_new(sizeof(UtX11Client), &object_interface);
  UtX11Client *self = (UtX11Client *)object;
  self->socket = ut_unix_domain_socket_client_new("/tmp/.X11-unix/X0");
  return object;
}

void ut_x11_client_connect(UtObject *object,
                           UtX11ClientConnectCallback callback, void *user_data,
                           UtObject *cancel) {
  assert(ut_object_is_x11_client(object));
  UtX11Client *self = (UtX11Client *)object;

  assert(callback != NULL);

  assert(self->connect_callback == NULL);
  self->connect_callback = callback;
  self->connect_user_data = user_data;
  self->connect_cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;

  ut_unix_domain_socket_client_connect(self->socket);
  ut_input_stream_read(self->socket, read_cb, self, self->read_cancel);

  UtObjectRef setup = ut_uint8_array_new();
  write_card8(setup, 0x6c); // Little endian.
  write_padding(setup, 1);
  write_card16(setup, 11); // Protocol major version.
  write_card16(setup, 0);  // Protocol minor version.
  write_card16(setup, 0);  // Authorizaton protocol name length.
  write_card16(setup, 0);  // Authorizaton protocol data length.
  write_padding(setup, 2);
  // Authorization protocol name.
  write_align_padding(setup, 4);
  // Authorization protocol data.
  write_align_padding(setup, 4);

  ut_output_stream_write(self->socket, setup);
}

uint32_t ut_x11_client_create_window(UtObject *object, int16_t x, int16_t y,
                                     uint16_t width, uint16_t height) {
  UtX11Client *self = (UtX11Client *)object;

  uint32_t id = create_resource_id(self);

  assert(self->screens_length > 0);
  X11Screen *screen = self->screens[0];

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, id);
  write_card32(request, screen->root); // parent
  write_int16(request, x);
  write_int16(request, y);
  write_card16(request, width);
  write_card16(request, height);
  write_card16(request, 0); // border_width
  write_card16(request, WINDOW_CLASS_INPUT_OUTPUT);
  write_card32(request, screen->root_visual->id);
  write_card32(request, VALUE_MASK_EVENT_MASK);
  write_card32(request, EVENT_KEY_PRESS | EVENT_KEY_RELEASE |
                            EVENT_BUTTON_PRESS | EVENT_BUTTON_RELEASE |
                            EVENT_ENTER_WINDOW | EVENT_LEAVE_WINDOW |
                            EVENT_POINTER_MOTION | EVENT_EXPOSURE |
                            EVENT_STRUCTURE_NOTIFY | EVENT_PROPERTY_CHANGE);

  send_request(self, 1, screen->root_visual->depth, request);

  return id;
}

void ut_x11_client_destroy_window(UtObject *object, uint32_t window) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, window);

  send_request(self, 4, 0, request);
}

void ut_x11_client_map_window(UtObject *object, uint32_t window) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, window);

  send_request(self, 8, 0, request);
}

void ut_x11_client_unmap_window(UtObject *object, uint32_t window) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, window);

  send_request(self, 10, 0, request);
}

void ut_x11_client_configure_window(UtObject *object, uint32_t window,
                                    int16_t x, int16_t y, uint16_t width,
                                    uint16_t height) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, window);
  write_card16(request, 0x0001 | 0x0002 | 0x0004 | 0x0008);
  write_padding(request, 2);
  write_value_int16(request, x);
  write_value_int16(request, y);
  write_value_card16(request, width);
  write_value_card16(request, height);

  send_request(self, 12, 0, request);
}

void ut_x11_client_intern_atom(UtObject *object, const char *name,
                               bool only_if_exists,
                               UtX11InternAtomCallback callback,
                               void *user_data, UtObject *cancel) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card16(request, strlen(name));
  write_padding(request, 2);
  write_string8(request, name);
  write_align_padding(request, 4);

  send_request_with_reply(self, 16, only_if_exists ? 1 : 0, request,
                          decode_intern_atom_reply, (RequestCallback)callback,
                          user_data, cancel);
}

void ut_x11_client_get_atom_name(UtObject *object, uint32_t atom,
                                 UtX11GetAtomNameCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, atom);

  send_request_with_reply(self, 17, 0, request, decode_get_atom_name_reply,
                          (RequestCallback)callback, user_data, cancel);
}

void ut_x11_client_change_property(UtObject *object, uint32_t window,
                                   uint32_t property, uint32_t type) {}

void ut_x11_client_delete_property(UtObject *object, uint32_t window,
                                   uint32_t property) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, window);
  write_card32(request, property);

  send_request(self, 19, 0, request);
}

void ut_x11_client_get_property(UtObject *object, uint32_t window,
                                uint32_t property,
                                UtX11GetPropertyCallback callback,
                                void *user_data, UtObject *cancel) {
  return ut_x11_client_get_property_full(object, window, property, 0, 0,
                                         0xffffffff, false, callback, user_data,
                                         cancel);
}

void ut_x11_client_get_property_full(UtObject *object, uint32_t window,
                                     uint32_t property, uint32_t type,
                                     uint32_t long_offset, uint32_t long_length,
                                     bool delete,
                                     UtX11GetPropertyCallback callback,
                                     void *user_data, UtObject *cancel) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, window);
  write_card32(request, property);
  write_card32(request, type);
  write_card32(request, long_offset);
  write_card32(request, long_length);

  send_request_with_reply(self, 31, delete ? 1 : 0, request,
                          decode_get_property_reply, (RequestCallback)callback,
                          user_data, cancel);
}

void ut_x11_client_list_properties(UtObject *object, uint32_t window,
                                   UtX11ListPropertiesCallback callback,
                                   void *user_data, UtObject *cancel) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, window);

  send_request_with_reply(self, 32, 0, request, decode_list_properties_reply,
                          (RequestCallback)callback, user_data, cancel);
}

uint32_t ut_x11_client_create_pixmap(UtObject *object, uint32_t drawable,
                                     uint16_t width, uint16_t height,
                                     uint8_t depth) {
  UtX11Client *self = (UtX11Client *)object;

  uint32_t id = create_resource_id(self);

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, id);
  write_card32(request, drawable);
  write_card16(request, width);
  write_card16(request, height);

  send_request(self, 53, depth, request);

  return id;
}

void ut_x11_client_free_pixmap(UtObject *object, uint32_t pixmap) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card32(request, pixmap);

  send_request(self, 54, 0, request);
}

void ut_x11_client_query_extension(UtObject *object, const char *name,
                                   UtX11QueryExtensionCallback callback,
                                   void *user_data, UtObject *cancel) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  write_card16(request, strlen(name));
  write_padding(request, 2);
  write_string8(request, name);
  write_align_padding(request, 4);

  send_request_with_reply(self, 98, 0, request, decode_query_extension_reply,
                          (RequestCallback)callback, user_data, cancel);
}

void ut_x11_client_list_extensions(UtObject *object,
                                   UtX11ListExtensionsCallback callback,
                                   void *user_data, UtObject *cancel) {
  UtX11Client *self = (UtX11Client *)object;

  UtObjectRef request = ut_uint8_array_new();
  send_request_with_reply(self, 99, 0, request, decode_list_extensions_reply,
                          (RequestCallback)callback, user_data, cancel);
}

bool ut_object_is_x11_client(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
