#include <assert.h>

#include "ut-curve25519.h"
#include "ut-uint256.h"

typedef struct {
  UtObject object;
} UtCurve25519;

static void pow25519(UtObject *a, UtObject *v) { ut_uint256_set(a, v); }

static UtObjectInterface object_interface = {.type_name = "UtCurve25519"};

UtObject *ut_curve25519_new() {
  UtObject *object = ut_object_new(sizeof(UtCurve25519), &object_interface);
  return object;
}

UtObject *ut_curve25519_multiply(UtObject *object, UtObject *k, UtObject *u) {
  assert(ut_object_is_curve25519(object));
  assert(ut_object_is_uint256(k));
  assert(ut_object_is_uint256(u));

  UtObjectRef a24 = ut_uint256_new(121665);

  UtObjectRef x1 = ut_uint256_copy(u);
  UtObjectRef x2 = ut_uint256_new(1);
  UtObjectRef z2 = ut_uint256_new(0);
  UtObjectRef x3 = ut_uint256_copy(u);
  UtObjectRef z3 = ut_uint256_new(1);

  uint8_t swap = 0;
  UtObjectRef a = ut_uint256_new(0);
  UtObjectRef aa = ut_uint256_new(0);
  UtObjectRef b = ut_uint256_new(0);
  UtObjectRef bb = ut_uint256_new(0);
  UtObjectRef e = ut_uint256_new(0);
  UtObjectRef c = ut_uint256_new(0);
  UtObjectRef d = ut_uint256_new(0);
  UtObjectRef da = ut_uint256_new(0);
  UtObjectRef cb = ut_uint256_new(0);
  UtObjectRef t = ut_uint256_new(0);
  UtObjectRef t2 = ut_uint256_new(0);
  for (int i = 254; i >= 0; i--) {
    uint8_t k_t = ut_uint256_get_bit(k, i);
    swap ^= k_t;
    ut_uint256_cswap(x2, swap, x3);
    ut_uint256_cswap(z2, swap, z3);
    k_t = swap;

    // A = x_2 + z_2
    // AA = A^2
    ut_uint256_add(a, x2, z2);
    ut_uint256_mul(aa, a, a);

    // B = x_2 - z_2
    // BB = B^2
    ut_uint256_sub(b, x2, z2);
    ut_uint256_mul(bb, b, b);

    // E = AA - BB
    // C = x_3 + z_3
    // D = x_3 - z_3
    ut_uint256_sub(e, aa, bb);
    ut_uint256_add(c, x3, z3);
    ut_uint256_sub(d, x3, z3);

    // DA = D * A
    // CB = C * B
    ut_uint256_mul(da, d, a);
    ut_uint256_mul(cb, c, b);

    // x_3 = (DA + CB)^2
    ut_uint256_add(t, da, cb);
    ut_uint256_mul(x3, t, t);

    // z_3 = x_1 * (DA - CB)^2
    ut_uint256_sub(t, da, cb);
    ut_uint256_mul(t2, t, t);
    ut_uint256_mul(z3, x1, t2);

    // x_2 = AA * BB
    ut_uint256_mul(x2, aa, bb);

    // z_2 = E * (AA + a24 * E)
    ut_uint256_mul(t, a24, e);
    ut_uint256_add(t2, aa, t);
    ut_uint256_mul(z2, e, t2);
  }

  ut_uint256_cswap(x2, swap, x3);
  ut_uint256_cswap(z2, swap, z3);

  // Return x_2 * (z_2^(p - 2))
  pow25519(t, z2);
  UtObject *u_out = ut_uint256_new(0);
  ut_uint256_mul(u_out, x2, t);

  return u_out;
}

bool ut_object_is_curve25519(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
