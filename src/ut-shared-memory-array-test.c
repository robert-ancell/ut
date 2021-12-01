#include <assert.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef array = ut_shared_memory_array_new(256);
  assert(ut_list_get_length(array) == 256);
  uint8_t *data = ut_shared_memory_array_get_data(array);
  for (size_t i = 0; i < 256; i++) {
    data[i] = i;
  }

  UtObjectRef sublist = ut_list_get_sublist(array, 128, 8);
  for (size_t i = 0; i < 8; i++) {
    assert(ut_uint8_list_get_element(sublist, i) == 128 + i);
  }

  return 0;
}
