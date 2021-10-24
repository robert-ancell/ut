#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef string = ut_constant_utf8_string_new("Hello");
  UtObjectRef code_points = ut_string_get_code_points(string);
  assert(ut_list_get_length(code_points) == 5);
  assert(ut_uint32_list_get_element(code_points, 0) == 'H');
  assert(ut_uint32_list_get_element(code_points, 1) == 'e');
  assert(ut_uint32_list_get_element(code_points, 2) == 'l');
  assert(ut_uint32_list_get_element(code_points, 3) == 'l');
  assert(ut_uint32_list_get_element(code_points, 4) == 'o');

  return 0;
}
