#include <assert.h>
#include <string.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef n0 = ut_uint256_new(0);
  UtObjectRef n1 = ut_uint256_new(1);
  UtObjectRef big0 = ut_uint256_new(12345678901234567890u);
  UtObjectRef big1 = ut_uint256_new(11223344556677889900u);
  UtObjectRef max32 = ut_uint256_new(0xffffffff);
  UtObjectRef max64 = ut_uint256_new(0xffffffffffffffff);
  UtObjectRef max128 = ut_uint256_new_from_data(
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
  UtObjectRef max256 = ut_uint256_new_from_data(
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
  UtObjectRef big256 = ut_uint256_new_from_data(
      0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0xf0, 0xde, 0xbc, 0x9a,
      0x78, 0x56, 0x34, 0x12, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
      0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12);

  assert(ut_uint256_is_uint64(n0));
  assert(ut_uint256_to_uint64(n0) == 0);
  ut_cstring_ref s0 = ut_object_to_string(n0);
  ut_assert_cstring_equal(s0, "<uint256>(0)");

  assert(ut_uint256_is_uint64(n1));
  assert(ut_uint256_to_uint64(n1) == 1);
  ut_cstring_ref s1 = ut_object_to_string(n1);
  ut_assert_cstring_equal(s1, "<uint256>(1)");

  assert(ut_uint256_is_uint64(max32));
  assert(ut_uint256_to_uint64(max32) == 0xffffffff);
  ut_cstring_ref s2 = ut_object_to_string(max32);
  ut_assert_cstring_equal(s2, "<uint256>(4294967295)");

  assert(ut_uint256_is_uint64(max64));
  assert(ut_uint256_to_uint64(max64) == 0xffffffffffffffff);
  ut_cstring_ref s3 = ut_object_to_string(max64);
  ut_assert_cstring_equal(s3, "<uint256>(18446744073709551615)");

  assert(!ut_uint256_is_uint64(max128));
  ut_cstring_ref s4 = ut_object_to_string(max128);
  ut_assert_cstring_equal(
      s4,
      "<uint256>("
      "0x00000000000000000000000000000000ffffffffffffffffffffffffffffffff)");

  assert(!ut_uint256_is_uint64(max256));
  ut_cstring_ref s5 = ut_object_to_string(max256);
  ut_assert_cstring_equal(
      s5,
      "<uint256>("
      "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)");

  UtObjectRef add01 = ut_uint256_new(0);
  ut_uint256_add(add01, n0, n1);
  assert(ut_uint256_is_uint64(add01));
  assert(ut_uint256_to_uint64(add01) == 1);

  UtObjectRef add10 = ut_uint256_new(0);
  ut_uint256_add(add10, n1, n0);
  assert(ut_uint256_is_uint64(add10));
  assert(ut_uint256_to_uint64(add10) == 1);

  UtObjectRef n12340000 = ut_uint256_new(12340000);
  UtObjectRef n5678 = ut_uint256_new(5678);
  UtObjectRef add12345678 = ut_uint256_new(0);
  ut_uint256_add(add12345678, n12340000, n5678);
  assert(ut_uint256_is_uint64(add12345678));
  assert(ut_uint256_to_uint64(add12345678) == 12345678);

  UtObjectRef addmax1 = ut_uint256_new(0);
  ut_uint256_add(addmax1, max256, n1);
  assert(ut_uint256_is_uint64(addmax1));
  assert(ut_uint256_to_uint64(addmax1) == 0);

  UtObjectRef addbig = ut_uint256_new(0);
  ut_uint256_add(addbig, big0, big1);
  assert(!ut_uint256_is_uint64(addbig));
  ut_cstring_ref saddbig = ut_object_to_string(addbig);
  ut_assert_cstring_equal(
      saddbig,
      "<uint256>("
      "0x0000000000000000000000000000000000000000000000014715fdf5ffb88a3e)");

  UtObjectRef mul10 = ut_uint256_new(0);
  ut_uint256_mul(mul10, n1, n0);
  assert(ut_uint256_is_uint64(mul10));
  assert(ut_uint256_to_uint64(mul10) == 0);

  UtObjectRef mul01 = ut_uint256_new(0);
  ut_uint256_mul(mul01, n0, n1);
  assert(ut_uint256_is_uint64(mul01));
  assert(ut_uint256_to_uint64(mul01) == 0);

  UtObjectRef mulbig = ut_uint256_new(0);
  ut_uint256_mul(mulbig, big0, big1);
  assert(!ut_uint256_is_uint64(mulbig));
  ut_cstring_ref smulbig = ut_object_to_string(mulbig);
  ut_assert_cstring_equal(
      smulbig,
      "<uint256>("
      "0x00000000000000000000000000000000683da5fa40be2e068d05ba9f76f8be98)");

  UtObjectRef mulbig256 = ut_uint256_new(0);
  ut_uint256_mul(mulbig256, big256, big256);
  assert(!ut_uint256_is_uint64(mulbig256));
  ut_cstring_ref smulbig256 = ut_object_to_string(mulbig256);
  ut_assert_cstring_equal(
      smulbig256,
      "<uint256>("
      "0x9b6a56d866788a95f43ce76b3fdcbcb94d0f77fe1940eedca5e20890f2a52100)");

  return 0;
}
