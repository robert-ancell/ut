#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-map-item.h"
#include "ut-map.h"
#include "ut-mutable-list.h"
#include "ut-mutable-string.h"
#include "ut-object-private.h"
#include "ut-string.h"
#include "ut-xml-document.h"
#include "ut-xml-element.h"

typedef struct {
  UtObject object;
  UtObject *root;
} UtXmlDocument;

static UtObject *decode_element(const char *text, size_t *offset);

static bool is_whitespace(uint32_t c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static bool is_name_start_char(uint32_t c) {
  return c == ':' || (c >= 'A' && c <= 'Z') || c == '_' ||
         (c >= 'a' && c <= 'z') || (c >= 0xc0 && c <= 0xd6) ||
         (c >= 0xd8 && c <= 0xf6) || (c >= 0xf8 && c <= 0x2ff) ||
         (c >= 0x370 && c <= 0x37d) || (c >= 0x37f && c <= 0x1fff) ||
         (c >= 0x200c && c <= 0x200d) || (c >= 0x2070 && c <= 0x218f) ||
         (c >= 0x2c00 && c <= 0x2fef) || (c >= 0x3001 && c <= 0xd7ff) ||
         (c >= 0xf900 && c <= 0xfdcf) || (c >= 0xfdf0 && c <= 0xfffd) ||
         (c >= 0x10000 && c <= 0xeffff);
}

static bool is_name_char(uint32_t c) {
  return is_name_start_char(c) || c == '-' || c == '.' ||
         (c >= '0' && c <= '9') || c == 0xb7 || (c >= 0x300 && c <= 0x36f) ||
         (c >= 0x203f && c <= 0x2040);
}

static void decode_whitespace(const char *text, size_t *offset) {
  while (is_whitespace(text[*offset])) {
    (*offset)++;
  }
}

static char *decode_name(const char *text, size_t *offset) {
  size_t end = *offset;
  if (!is_name_start_char(text[end])) {
    return NULL;
  }
  end++;

  while (is_name_char(text[end])) {
    end++;
  }

  char *name = strndup(text + *offset, end - *offset);
  *offset = end;
  return name;
}

static UtObject *decode_character_data(const char *text, size_t *offset) {
  // FIXME: Needs work
  size_t start = *offset, end = start;
  while (text[end] != '\0' && text[end] != '<' && text[end] != '&') {
    end++;
  }

  if (start == end) {
    return NULL;
  }

  *offset = end;
  return ut_string_new_sized(text + start, end - start);
}

static UtObject *decode_content(const char *text, size_t *offset) {
  UtObjectRef content = ut_mutable_list_new();

  size_t end = *offset;
  UtObjectRef leading_text = decode_character_data(text, &end);
  if (leading_text != NULL) {
    ut_mutable_list_append(content, leading_text);
  }
  while (true) {
    UtObjectRef element = decode_element(text, &end);
    if (element == NULL) {
      *offset = end;
      return ut_list_get_length(content) == 0 ? NULL : ut_object_ref(content);
    }

    ut_mutable_list_append(content, element);

    UtObjectRef trailing_text = decode_character_data(text, &end);
    if (trailing_text != NULL) {
      ut_mutable_list_append(content, trailing_text);
    }
  }
}

static char *decode_attribute_value(const char *text, size_t *offset) {
  size_t end = *offset;
  if (text[end] != '"') {
    return NULL;
  }
  end++;

  size_t start = end;
  while (true) {
    if (text[end] == '\0') {
      return NULL;
    } else if (text[end] == '"') {
      *offset = end + 1;
      return strndup(text + start, end - start);
    }
    end++;
  }
}

static bool decode_attribute(const char *text, size_t *offset, char **name,
                             char **value) {
  size_t end = *offset;
  char *name_ = decode_name(text, &end);
  if (name_ == NULL) {
    return false;
  }

  decode_whitespace(text, &end);
  if (text[end] != '=') {
    free(name_);
    return false;
  }
  end++;
  decode_whitespace(text, &end);
  char *value_ = decode_attribute_value(text, &end);
  if (value_ == NULL) {
    free(name_);
    return false;
  }

  *name = name_;
  *value = value_;
  *offset = end;
  return true;
}

static char *decode_start_tag(const char *text, size_t *offset,
                              UtObject **attributes, bool *is_empty) {
  size_t end = *offset;
  if (text[end] != '<') {
    return NULL;
  }
  end++;

  char *tag_name = decode_name(text, &end);
  if (tag_name == NULL) {
    return NULL;
  }

  UtObjectRef attributes_ = ut_map_new();
  while (true) {
    decode_whitespace(text, &end);

    char *name, *value;
    if (!decode_attribute(text, &end, &name, &value)) {
      break;
    }
    ut_map_insert_string_take(attributes_, name, ut_string_new(value));
    free(value);
  }

  decode_whitespace(text, &end);

  bool is_empty_ = false;
  if (text[end] == '/') {
    is_empty_ = true;
    end++;
  }

  if (text[end] != '>') {
    free(tag_name);
    return NULL;
  }
  end++;

  *offset = end;

  *attributes = ut_object_ref(attributes_);
  *is_empty = is_empty_;
  return tag_name;
}

static char *decode_end_tag(const char *text, size_t *offset) {
  size_t end = *offset;
  if (text[end] != '<' || text[end + 1] != '/') {
    return NULL;
  }
  end += 2;

  char *name = decode_name(text, &end);
  if (name == NULL) {
    return NULL;
  }

  if (text[end] != '>') {
    free(name);
    return NULL;
  }
  end++;

  *offset = end;

  return name;
}

static UtObject *decode_element(const char *text, size_t *offset) {
  size_t end = *offset;
  UtObjectRef attributes = NULL;
  bool is_empty = false;
  char *name = decode_start_tag(text, &end, &attributes, &is_empty);
  if (name == NULL) {
    return NULL;
  }
  UtObjectRef content = NULL;
  if (!is_empty) {
    content = decode_content(text, &end);
    char *end_name = decode_end_tag(text, &end);
    if (!end_name) {
      free(name);
      return NULL;
    }
    bool name_matches = strcmp(name, end_name) == 0;
    free(end_name);
    if (!name_matches) {
      free(name);
      return NULL;
    }
  }

  UtObject *element = ut_xml_element_new(name, attributes, content);
  free(name);
  *offset = end;
  return element;
}

static bool decode_document(const char *text, size_t *offset, UtObject **root) {
  size_t end = *offset;
  UtObjectRef root_ = decode_element(text, &end);
  if (root_ == NULL) {
    return false;
  }

  *root = ut_object_ref(root_);
  *offset = end;
  return true;
}

static bool encode_character_data(UtObject *buffer, const char *data) {
  ut_mutable_string_append(buffer, data);
  return true;
}

static bool encode_element(UtObject *buffer, UtObject *element) {
  const char *name = ut_xml_element_get_name(element);
  ut_mutable_string_append(buffer, "<");
  ut_mutable_string_append(buffer, name);
  UtObject *attributes = ut_xml_element_get_attributes(element);
  if (attributes != NULL) {
    UtObjectRef attribute_items = ut_map_get_items(attributes);
    size_t attributes_length = ut_list_get_length(attribute_items);
    for (size_t i = 0; i < attributes_length; i++) {
      UtObjectRef item = ut_list_get_element(attribute_items, i);
      UtObjectRef name = ut_map_item_get_key(item);
      UtObjectRef value = ut_map_item_get_value(item);
      ut_mutable_string_append(buffer, " ");
      ut_mutable_string_append(buffer, ut_string_get_text(name));
      ut_mutable_string_append(buffer, "=\"");
      ut_mutable_string_append(buffer, ut_string_get_text(value));
      ut_mutable_string_append(buffer, "\"");
    }
  }
  UtObject *content = ut_xml_element_get_content(element);
  size_t content_length = content != NULL ? ut_list_get_length(content) : 0;
  if (content_length == 0) {
    ut_mutable_string_append(buffer, "/>");
    return true;
  }
  ut_mutable_string_append(buffer, ">");
  for (size_t i = 0; i < content_length; i++) {
    UtObjectRef child = ut_list_get_element(content, i);
    if (ut_object_implements_string(child)) {
      if (!encode_character_data(buffer, ut_string_get_text(child))) {
        return false;
      }
    } else if (ut_object_is_xml_element(child)) {
      if (!encode_element(buffer, child)) {
        return false;
      }
    } else {
      assert(false);
    }
  }
  ut_mutable_string_append(buffer, "</");
  ut_mutable_string_append(buffer, name);
  ut_mutable_string_append(buffer, ">");

  return true;
}

static void ut_xml_document_init(UtObject *object) {
  UtXmlDocument *self = (UtXmlDocument *)object;
  self->root = NULL;
}

static void ut_xml_document_cleanup(UtObject *object) {
  UtXmlDocument *self = (UtXmlDocument *)object;
  ut_object_unref(self->root);
}

static UtObjectInterface object_interface = {.type_name = "UtXmlDocument",
                                             .init = ut_xml_document_init,
                                             .cleanup =
                                                 ut_xml_document_cleanup};

UtObject *ut_xml_document_new(UtObject *root) {
  assert(ut_object_is_xml_element(root));
  UtObject *object = ut_object_new(sizeof(UtXmlDocument), &object_interface);
  UtXmlDocument *self = (UtXmlDocument *)object;
  self->root = ut_object_ref(root);
  return object;
}

UtObject *ut_xml_document_new_from_text(const char *text) {
  UtObjectRef root = NULL;
  size_t end = 0;
  if (!decode_document(text, &end, &root)) {
    return NULL;
  }
  if (text[end] != '\0') {
    return NULL;
  }

  return ut_xml_document_new(root);
}

UtObject *ut_xml_document_get_root(UtObject *object) {
  assert(ut_object_is_xml_document(object));
  UtXmlDocument *self = (UtXmlDocument *)object;
  return self->root;
}

char *ut_xml_document_to_text(UtObject *object) {
  assert(ut_object_is_xml_document(object));
  UtXmlDocument *self = (UtXmlDocument *)object;
  UtObjectRef buffer = ut_mutable_string_new("");
  encode_element(buffer, self->root);
  return ut_string_take_text(buffer);
}

bool ut_object_is_xml_document(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
