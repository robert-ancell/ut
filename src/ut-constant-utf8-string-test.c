#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef string = ut_constant_utf8_string_new("Hello");
  UtObjectRef code_points = ut_string_get_code_points(string);
  const uint32_t *code_point_data = ut_uint32_list_get_data(code_points);
  assert(ut_list_get_length(code_points) == 5);
  assert(code_point_data[0] == 'H');
  assert(code_point_data[1] == 'e');
  assert(code_point_data[2] == 'l');
  assert(code_point_data[3] == 'l');
  assert(code_point_data[4] == 'o');

  return 0;
}
