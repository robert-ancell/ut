#include <assert.h>

#include "ut-cstring.h"
#include "ut-uint256.h"

typedef struct {
  UtObject object;
  uint32_t value[8];
} UtUint256;

/// Set [out] = [a] + [b].
static void add(uint32_t *out, uint32_t *a, uint32_t *b) {
  uint64_t carry = 0;
  for (size_t i = 0; i < 8; i++) {
    uint64_t sum = (uint64_t)a[i] + (uint64_t)b[i] + carry;
    out[i] = sum & 0xffffffff;
    carry = sum >> 32;
  }
}

/// Set [out] to the twos complement (i.e. negative value) of [value].
static void twos_complement(uint32_t *out, uint32_t *value) {
  for (size_t i = 0; i < 8; i++) {
    out[i] = ~value[i];
  }
  uint32_t one[8] = {1, 0, 0, 0, 0, 0, 0, 0};
  add(out, out, one);
}

static char *ut_uint256_to_string(UtObject *object) {
  uint32_t *v = ((UtUint256 *)object)->value;
  if (ut_uint256_is_uint64(object)) {
    return ut_cstring_new_printf("<uint256>(%lu)",
                                 ut_uint256_to_uint64(object));
  } else {
    return ut_cstring_new_printf(
        "<uint256>(0x%08x%08x%08x%08x%08x%08x%08x%08x)", v[7], v[6], v[5], v[4],
        v[3], v[2], v[1], v[0]);
  }
}

static bool ut_uint256_equal(UtObject *object, UtObject *other) {
  uint32_t *v = ((UtUint256 *)object)->value;
  if (!ut_object_is_uint256(other)) {
    return false;
  }
  uint32_t *other_v = ((UtUint256 *)other)->value;

  for (size_t i = 0; i < 8; i++) {
    if (v[i] != other_v[i]) {
      return false;
    }
  }

  return true;
}

static int ut_uint256_hash(UtObject *object) {
  uint32_t *v = ((UtUint256 *)object)->value;
  // FIXME: Use a better algorithm
  return v[0] ^ v[1] ^ v[2] ^ v[3] ^ v[4] ^ v[5] ^ v[6] ^ v[7];
}

static UtObjectInterface object_interface = {.type_name = "UtUint256",
                                             .to_string = ut_uint256_to_string,
                                             .equal = ut_uint256_equal,
                                             .hash = ut_uint256_hash};

UtObject *ut_uint256_new(uint64_t value) {
  UtObject *object = ut_object_new(sizeof(UtUint256), &object_interface);
  uint32_t *v = ((UtUint256 *)object)->value;
  v[0] = value & 0xffffffff;
  v[1] = value >> 32;
  v[2] = 0;
  v[3] = 0;
  v[4] = 0;
  v[5] = 0;
  v[6] = 0;
  v[7] = 0;
  return object;
}

UtObject *ut_uint256_new_from_data(
    uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5,
    uint8_t v6, uint8_t v7, uint8_t v8, uint8_t v9, uint8_t v10, uint8_t v11,
    uint8_t v12, uint8_t v13, uint8_t v14, uint8_t v15, uint8_t v16,
    uint8_t v17, uint8_t v18, uint8_t v19, uint8_t v20, uint8_t v21,
    uint8_t v22, uint8_t v23, uint8_t v24, uint8_t v25, uint8_t v26,
    uint8_t v27, uint8_t v28, uint8_t v29, uint8_t v30, uint8_t v31) {
  UtObject *object = ut_object_new(sizeof(UtUint256), &object_interface);
  uint32_t *v = ((UtUint256 *)object)->value;
  v[0] = (uint32_t)v0 | (uint32_t)v1 << 8 | (uint32_t)v2 << 16 |
         (uint32_t)v3 << 24;
  v[1] = (uint32_t)v4 | (uint32_t)v5 << 8 | (uint32_t)v6 << 16 |
         (uint32_t)v7 << 24;
  v[2] = (uint32_t)v8 | (uint32_t)v9 << 8 | (uint32_t)v10 << 16 |
         (uint32_t)v11 << 24;
  v[3] = (uint32_t)v12 | (uint32_t)v13 << 8 | (uint32_t)v14 << 16 |
         (uint32_t)v15 << 24;
  v[4] = (uint32_t)v16 | (uint32_t)v17 << 8 | (uint32_t)v18 << 16 |
         (uint32_t)v19 << 24;
  v[5] = (uint32_t)v20 | (uint32_t)v21 << 8 | (uint32_t)v22 << 16 |
         (uint32_t)v23 << 24;
  v[6] = (uint32_t)v24 | (uint32_t)v25 << 8 | (uint32_t)v26 << 16 |
         (uint32_t)v27 << 24;
  v[7] = (uint32_t)v28 | (uint32_t)v29 << 8 | (uint32_t)v30 << 16 |
         (uint32_t)v31 << 24;
  return object;
}

UtObject *ut_uint256_copy(UtObject *object) {
  assert(ut_object_is_uint256(object));
  UtObject *copy = ut_object_new(sizeof(UtUint256), &object_interface);
  ut_uint256_set(copy, object);
  return copy;
}

// FIXME: Naming?
bool ut_uint256_is_uint64(UtObject *object) {
  assert(ut_object_is_uint256(object));
  uint32_t *v = ((UtUint256 *)object)->value;
  return v[7] == 0 && v[6] == 0 && v[5] == 0 && v[4] == 0 && v[3] == 0 &&
         v[2] == 0;
}

uint64_t ut_uint256_to_uint64(UtObject *object) {
  assert(ut_object_is_uint256(object));
  uint32_t *v = ((UtUint256 *)object)->value;

  return (uint64_t)v[1] << 32 | v[0];
}

void ut_uint256_set(UtObject *object, UtObject *value_) {
  assert(ut_object_is_uint256(object));
  assert(ut_object_is_uint256(value_));

  uint32_t *v = ((UtUint256 *)object)->value;
  uint32_t *value = ((UtUint256 *)value_)->value;

  for (size_t i = 0; i < 8; i++) {
    v[i] = value[i];
  }
}

void ut_uint256_add(UtObject *object, UtObject *a_, UtObject *b_) {
  assert(ut_object_is_uint256(object));
  assert(ut_object_is_uint256(a_));
  assert(ut_object_is_uint256(b_));
  assert(object != a_ && object != b_);

  uint32_t *v = ((UtUint256 *)object)->value;
  uint32_t *a = ((UtUint256 *)a_)->value;
  uint32_t *b = ((UtUint256 *)b_)->value;

  add(v, a, b);
}

void ut_uint256_sub(UtObject *object, UtObject *a_, UtObject *b_) {
  assert(ut_object_is_uint256(object));
  assert(ut_object_is_uint256(a_));
  assert(ut_object_is_uint256(b_));

  uint32_t *v = ((UtUint256 *)object)->value;
  uint32_t *a = ((UtUint256 *)a_)->value;
  uint32_t *b = ((UtUint256 *)b_)->value;

  uint32_t negb[8];
  twos_complement(negb, b);
  add(v, a, negb);
}

void ut_uint256_mul(UtObject *object, UtObject *a_, UtObject *b_) {
  assert(ut_object_is_uint256(object));
  assert(ut_object_is_uint256(a_));
  assert(ut_object_is_uint256(b_));

  uint32_t *v = ((UtUint256 *)object)->value;
  uint32_t *a = ((UtUint256 *)a_)->value;
  uint32_t *b = ((UtUint256 *)b_)->value;

  uint64_t m00 = (uint64_t)a[0] * (uint64_t)b[0];
  uint64_t m0 = m00;
  uint64_t c0 = m0 >> 32;
  v[0] = m0 & 0xffffffff;

  uint64_t m01 = (uint64_t)a[0] * (uint64_t)b[1];
  uint64_t m10 = (uint64_t)a[1] * (uint64_t)b[0];
  uint64_t m1 = m01 + m10;
  uint64_t c1 = m1 >> 32;
  uint64_t m1c = (m1 & 0xffffffff) + c0;
  c1 += m1c >> 32;
  v[1] = m1c & 0xffffffff;

  uint64_t m02 = (uint64_t)a[0] * (uint64_t)b[2];
  uint64_t m11 = (uint64_t)a[1] * (uint64_t)b[1];
  uint64_t m20 = (uint64_t)a[2] * (uint64_t)b[0];
  uint64_t c2 = (c1 >> 32) + (m02 >> 32) + (m11 >> 32) + (m20 >> 32);
  uint64_t m2 = (m02 & 0xffffffff) + (m11 & 0xffffffff) + (m20 & 0xffffffff);
  c2 += m2 >> 32;
  uint64_t m2c = (m2 & 0xffffffff) + (c1 & 0xffffffff);
  c2 += m2c >> 32;
  v[2] = m2c & 0xffffffff;

  uint64_t m03 = (uint64_t)a[0] * (uint64_t)b[3];
  uint64_t m12 = (uint64_t)a[1] * (uint64_t)b[2];
  uint64_t m21 = (uint64_t)a[2] * (uint64_t)b[1];
  uint64_t m30 = (uint64_t)a[3] * (uint64_t)b[0];
  uint64_t c3 =
      (c2 >> 32) + (m03 >> 32) + (m12 >> 32) + (m21 >> 32) + (m30 >> 32);
  uint64_t m3 = (m03 & 0xffffffff) + (m12 & 0xffffffff) + (m21 & 0xffffffff) +
                (m30 & 0xffffffff);
  c3 += m3 >> 32;
  uint64_t m3c = (m3 & 0xffffffff) + (c2 & 0xffffffff);
  c3 += m3c >> 32;
  v[3] = m3c & 0xffffffff;

  uint64_t m04 = (uint64_t)a[0] * (uint64_t)b[4];
  uint64_t m13 = (uint64_t)a[1] * (uint64_t)b[3];
  uint64_t m22 = (uint64_t)a[2] * (uint64_t)b[2];
  uint64_t m31 = (uint64_t)a[3] * (uint64_t)b[1];
  uint64_t m40 = (uint64_t)a[4] * (uint64_t)b[0];
  uint64_t c4 = (c3 >> 32) + (m04 >> 32) + (m13 >> 32) + (m22 >> 32) +
                (m31 >> 32) + (m40 >> 32);
  uint64_t m4 = (m04 & 0xffffffff) + (m13 & 0xffffffff) + (m22 & 0xffffffff) +
                (m31 & 0xffffffff) + (m40 & 0xffffffff);
  c4 += m4 >> 32;
  uint64_t m4c = (m4 & 0xffffffff) + (c3 & 0xffffffff);
  c4 += m4c >> 32;
  v[4] = m4c & 0xffffffff;

  uint64_t m05 = (uint64_t)a[0] * (uint64_t)b[5];
  uint64_t m14 = (uint64_t)a[1] * (uint64_t)b[4];
  uint64_t m23 = (uint64_t)a[2] * (uint64_t)b[3];
  uint64_t m32 = (uint64_t)a[3] * (uint64_t)b[2];
  uint64_t m41 = (uint64_t)a[4] * (uint64_t)b[1];
  uint64_t m50 = (uint64_t)a[5] * (uint64_t)b[0];
  uint64_t c5 = (c4 >> 32) + (m05 >> 32) + (m14 >> 32) + (m23 >> 32) +
                (m32 >> 32) + (m41 >> 32) + (m50 >> 32);
  uint64_t m5 = (m05 & 0xffffffff) + (m14 & 0xffffffff) + (m23 & 0xffffffff) +
                (m32 & 0xffffffff) + (m41 & 0xffffffff) + (m50 & 0xffffffff);
  c5 += m5 >> 32;
  uint64_t m5c = (m5 & 0xffffffff) + (c4 & 0xffffffff);
  c5 += m5c >> 32;
  v[5] = m5c & 0xffffffff;

  uint64_t m06 = (uint64_t)a[0] * (uint64_t)b[6];
  uint64_t m15 = (uint64_t)a[1] * (uint64_t)b[5];
  uint64_t m24 = (uint64_t)a[2] * (uint64_t)b[4];
  uint64_t m33 = (uint64_t)a[3] * (uint64_t)b[3];
  uint64_t m42 = (uint64_t)a[4] * (uint64_t)b[2];
  uint64_t m51 = (uint64_t)a[5] * (uint64_t)b[1];
  uint64_t m60 = (uint64_t)a[6] * (uint64_t)b[0];
  uint64_t c6 = (c5 >> 32) + (m06 >> 32) + (m15 >> 32) + (m24 >> 32) +
                (m33 >> 32) + (m42 >> 32) + (m51 >> 32) + (m60 >> 32);
  uint64_t m6 = (m06 & 0xffffffff) + (m15 & 0xffffffff) + (m24 & 0xffffffff) +
                (m33 & 0xffffffff) + (m42 & 0xffffffff) + (m51 & 0xffffffff) +
                (m60 & 0xffffffff);
  c6 += m6 >> 32;
  uint64_t m6c = (m6 & 0xffffffff) + (c5 & 0xffffffff);
  c6 += m6c >> 32;
  v[6] = m6c & 0xffffffff;

  uint64_t m07 = (uint64_t)a[0] * (uint64_t)b[7];
  uint64_t m16 = (uint64_t)a[1] * (uint64_t)b[6];
  uint64_t m25 = (uint64_t)a[2] * (uint64_t)b[5];
  uint64_t m34 = (uint64_t)a[3] * (uint64_t)b[4];
  uint64_t m43 = (uint64_t)a[4] * (uint64_t)b[3];
  uint64_t m52 = (uint64_t)a[5] * (uint64_t)b[2];
  uint64_t m61 = (uint64_t)a[6] * (uint64_t)b[1];
  uint64_t m70 = (uint64_t)a[7] * (uint64_t)b[0];
  uint64_t m7 = (m07 & 0xffffffff) + (m16 & 0xffffffff) + (m25 & 0xffffffff) +
                (m34 & 0xffffffff) + (m43 & 0xffffffff) + (m52 & 0xffffffff) +
                (m61 & 0xffffffff) + (m70 & 0xffffffff);
  uint64_t m7c = (m7 & 0xffffffff) + (c6 & 0xffffffff);
  v[7] = m7c & 0xffffffff;
}

bool ut_object_is_uint256(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
