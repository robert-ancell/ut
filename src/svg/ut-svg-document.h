#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_svg_document_new(UtObject *xml_document);

UtObject *ut_svg_document_new_from_text(const char *text);

bool ut_object_is_svg_document(UtObject *object);
