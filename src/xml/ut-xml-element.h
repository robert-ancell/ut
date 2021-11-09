#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_xml_element_new(const char *name, UtObject *attributes,
                             UtObject *content);

const char *ut_xml_element_get_name(UtObject *object);

UtObject *ut_xml_element_get_attributes(UtObject *object);

UtObject *ut_xml_element_get_content(UtObject *object);

bool ut_object_is_xml_element(UtObject *object);
