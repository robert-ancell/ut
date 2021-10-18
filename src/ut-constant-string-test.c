#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObject *string = ut_constant_string_new("Hello");
  UtObject *code_points = ut_string_get_code_points(string);
  const uint32_t *code_point_data = ut_uint32_list_get_data(code_points);
  printf("code points:");
  for (size_t i = 0; i < ut_list_get_length(code_points); i++) {
    printf(" %x", code_point_data[i]);
  }
  printf("\n");

  return 0;
}
