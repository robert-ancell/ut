#include <assert.h>
#include <string.h>

#include "ut-svg-document.h"
#include "xml/ut-xml-document.h"
#include "xml/ut-xml-element.h"

typedef struct {
  UtObject object;
} UtSvgDocument;

static bool parse_document(UtSvgDocument *self, UtObject *xml_document) {
  UtObject *root = ut_xml_document_get_root(xml_document);

  if (strcmp(ut_xml_element_get_name(root), "svg") != 0) {
    return false;
  }

  return true;
}

static UtObjectInterface object_interface = {.type_name = "UtSvgDocument"};

UtObject *ut_svg_document_new(UtObject *xml_document) {
  assert(ut_object_is_xml_document(xml_document));
  UtObject *object = ut_object_new(sizeof(UtSvgDocument), &object_interface);
  UtSvgDocument *self = (UtSvgDocument *)object;
  assert(parse_document(self, xml_document));
  return object;
}

UtObject *ut_svg_document_new_from_text(const char *text) {
  UtObjectRef xml_document = ut_xml_document_new_from_text(text);
  return ut_svg_document_new(xml_document);
}

bool ut_object_is_svg_document(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
