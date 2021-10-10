#include <stdint.h>
#include <stdio.h>

#include "ut-bytes.h"
#include "ut-list.h"

int main(int argc, char **argv) {
  const uint8_t data[] = {0x01, 0x02, 0x03};
  UtObject *bytes = ut_bytes_new(data, 3);
  printf("%zi\n", ut_list_get_length(bytes));
  ut_object_unref(bytes);

  return 0;
}
