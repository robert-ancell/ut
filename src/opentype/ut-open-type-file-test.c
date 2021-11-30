#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef file = ut_memory_mapped_file_new(
      "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf");
  ut_file_open_read(file);
  UtObjectRef font = ut_open_type_file_new(file);
  ut_open_type_file_open(font);

  ut_event_loop_run();

  return 0;
}
