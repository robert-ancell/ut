#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut.h"

static void test_encode() {
  UtObjectRef empty_element_root = ut_xml_element_new("tag", NULL, NULL);
  UtObjectRef empty_element_document = ut_xml_document_new(empty_element_root);
  ut_cstring empty_element_text =
      ut_xml_document_to_text(empty_element_document);
  assert(strcmp(empty_element_text, "<tag/>") == 0);

  UtObjectRef attribute_attributes = ut_map_new();
  ut_map_insert_string_take(attribute_attributes, "name",
                            ut_string_new("value"));
  UtObjectRef attribute_root =
      ut_xml_element_new("tag", attribute_attributes, NULL);
  UtObjectRef attribute_document = ut_xml_document_new(attribute_root);
  ut_cstring attribute_text = ut_xml_document_to_text(attribute_document);
  assert(strcmp(attribute_text, "<tag name=\"value\"/>") == 0);

  UtObjectRef multiple_attribute_attributes = ut_map_new();
  ut_map_insert_string_take(multiple_attribute_attributes, "name1",
                            ut_string_new("value1"));
  ut_map_insert_string_take(multiple_attribute_attributes, "name2",
                            ut_string_new("value2"));
  UtObjectRef multiple_attribute_root =
      ut_xml_element_new("tag", multiple_attribute_attributes, NULL);
  UtObjectRef multiple_attribute_document =
      ut_xml_document_new(multiple_attribute_root);
  ut_cstring multiple_attribute_text =
      ut_xml_document_to_text(multiple_attribute_document);
  assert(strcmp(multiple_attribute_text,
                "<tag name1=\"value1\" name2=\"value2\"/>") == 0);

  UtObjectRef content_content = ut_list_new();
  ut_mutable_list_append_take(content_content, ut_string_new("Hello World!"));
  UtObjectRef content_root = ut_xml_element_new("tag", NULL, content_content);
  UtObjectRef content_document = ut_xml_document_new(content_root);
  ut_cstring content_text = ut_xml_document_to_text(content_document);
  assert(strcmp(content_text, "<tag>Hello World!</tag>") == 0);
}

static void test_decode() {
  UtObjectRef empty_document = ut_xml_document_new_from_text("");
  assert(empty_document == NULL);

  UtObjectRef empty_element_document = ut_xml_document_new_from_text("<tag/>");
  assert(empty_element_document != NULL);
  assert(strcmp(ut_xml_element_get_name(
                    ut_xml_document_get_root(empty_element_document)),
                "tag") == 0);

  UtObjectRef tag_name_leading_whitespace_document =
      ut_xml_document_new_from_text("< tag/>");
  assert(tag_name_leading_whitespace_document == NULL);

  UtObjectRef tag_name_trailing_whitespace_document =
      ut_xml_document_new_from_text("<tag />");
  assert(tag_name_trailing_whitespace_document != NULL);
  assert(strcmp(ut_xml_element_get_name(ut_xml_document_get_root(
                    tag_name_trailing_whitespace_document)),
                "tag") == 0);

  UtObjectRef attribute_document =
      ut_xml_document_new_from_text("<tag name=\"value\"/>");
  assert(attribute_document != NULL);

  UtObjectRef multiple_attribute_document =
      ut_xml_document_new_from_text("<foo name1=\"value1\" name2=\"value2\"/>");
  assert(multiple_attribute_document != NULL);

  UtObjectRef no_content_document =
      ut_xml_document_new_from_text("<tag></tag>");
  assert(no_content_document != NULL);
  assert(strcmp(ut_xml_element_get_name(
                    ut_xml_document_get_root(no_content_document)),
                "tag") == 0);

  UtObjectRef content_document =
      ut_xml_document_new_from_text("<tag>Hello World!</tag>");
  assert(content_document != NULL);
  assert(strcmp(ut_xml_element_get_name(
                    ut_xml_document_get_root(content_document)),
                "tag") == 0);

  UtObjectRef mismatched_tags_document =
      ut_xml_document_new_from_text("<foo></bar>");
  assert(mismatched_tags_document == NULL);

  UtObjectRef interleaved_tags_document =
      ut_xml_document_new_from_text("<foo><bar></foo></bar>");
  assert(interleaved_tags_document == NULL);
}

int main(int argc, char **argv) {
  test_encode();
  test_decode();
}
