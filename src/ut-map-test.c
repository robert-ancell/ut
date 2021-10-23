#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef map = ut_map_new();
  ut_map_insert_string_take(map, "one", ut_uint8_new(1));
  ut_map_insert_string_take(map, "two", ut_uint8_new(42));
  ut_map_insert_string_take(map, "two", ut_uint8_new(2));
  ut_cstring map_string = ut_object_to_string(map);
  printf("map: %s\n", map_string);

  return 0;
}
