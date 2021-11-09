#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_xml_document_new(UtObject *root);

UtObject *ut_xml_document_new_from_text(const char *text);

UtObject *ut_xml_document_get_root(UtObject *object);

char *ut_xml_document_to_text(UtObject *object);

bool ut_object_is_xml_document(UtObject *object);
