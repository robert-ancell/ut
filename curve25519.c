#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Curve25519:
// https://datatracker.ietf.org/doc/html/rfc7748
//
// TLS 1.3:
// https://datatracker.ietf.org/doc/html/rfc8446

static uint8_t one[32] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t zero[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// a24 = 486662 - 2) / 4 = 121665
static uint8_t curve25519_a24[32] = {64, 1, 219, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// p = 2^255 - 19 = 
static uint8_t curve25519_p_minus_2[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void set(const uint8_t *in, uint8_t *out) {
  for (int i = 0; i < 32; i++) {
    out[i] = in[i];
  }
}

static void add(const uint8_t *a, const uint8_t *b, uint8_t *out) {
  uint8_t carry = 0;
  for (int i = 0; i < 32; i++) {
    uint16_t x = a[i] + b[i] + carry;
    out[i] = x & 0xff;
    carry = x >> 8;
  }
}

static void sub(const uint8_t *a, const uint8_t *b, uint8_t *out) {
  for (int i = 0; i < 32; i++) {
    out[i] = a[i] - b[i];
  }
}

static void mul(const uint8_t *a, const uint8_t *b, uint8_t *out) {
   
}

// Returns a^(p-2)
static void pow25519(const uint8_t *a, uint8_t *out) {
   set(a, out);
   mul(a, a, t);
}

static void cswap(bool swap, uint8_t *x2, uint8_t *x3) {
  uint8_t mask = swap ? 0xff : 0x00;
  for (int i = 0; i < 32; i++) {
    uint8_t dummy = mask & (x2[i] ^ x3[i]);
    x2[i] = x2[i] ^ dummy;
    x3[i] = x3[i] ^ dummy;
  }
}

static void x22519(const uint8_t *k, const uint8_t *u, uint8_t *u_out) {
  uint8_t x1[32], x2[32], z2[32], x3[32], z3[32];

  set(u, x1);
  set(one, x2);
  set(zero, z2);
  set(u, x3);
  set(zero, z3);

  uint8_t swap = 0;
  for (int i = 254; i >= 0; i--) {
    uint8_t a[32], aa[32], b[32], bb[32], e[32], c[32], d[32], da[32], cb[32],
        t[32], t2[32];

    uint8_t k_t = (k[i / 8] >> (i % 8)) & 0x1;
    swap ^= k_t;
    cswap(swap, x2, x3);
    cswap(swap, z2, z3);
    k_t = swap;

    // A = x_2 + z_2
    // AA = A^2
    add(x2, z2, a);
    mul(a, a, aa);

    // B = x_2 - z_2
    // BB = B^2
    sub(x2, z2, b);
    mul(b, b, bb);

    // E = AA - BB
    // C = x_3 + z_3
    // D = x_3 - z_3
    sub(aa, bb, e);
    add(x3, z3, c);
    sub(x3, z3, d);

    // DA = D * A
    // CB = C * B
    mul(d, a, da);
    mul(c, b, cb);

    // x_3 = (DA + CB)^2
    add(da, cb, t);
    mul(t, t, x3);

    // z_3 = x_1 * (DA - CB)^2
    sub(da, cb, t);
    mul(t, t, t2);
    mul(x1, t2, z3);

    // x_2 = AA * BB
    mul(aa, bb, x2);

    // z_2 = E * (AA + a24 * E)
    mul(curve25519_a24, e, t);
    add(aa, t, t2);
    mul(e, t2, z2);
  }

  cswap(swap, x2, x3);
  cswap(swap, z2, z3);

  // Return x_2 * (z_2^(p - 2))
  uint8_t t[32];
  pow25519(z2, t);
  mul(x2, t, u_out);
}

int main(int argc, char **argv) 
{
   uint8_t k[32] = {0xa5, 0x46, 0xe3, 0x6b, 0xf0, 0x52, 0x7c, 0x9d, 0x3b, 0x16, 0x15, 0x4b, 0x82, 0x46, 0x5e, 0xdd, 0x62, 0x14, 0x4c, 0x0a, 0xc1, 0xfc, 0x5a, 0x18, 0x50, 0x6a, 0x22, 0x44, 0xba, 0x44, 0x9a, 0xc4};
   uint8_t u[32] = {0xe6, 0xdb, 0x68, 0x67, 0x58, 0x30, 0x30, 0xdb, 0x35, 0x94, 0xc1, 0xa4, 0x24, 0xb1, 0x5f, 0x7c, 0x72, 0x66, 0x24, 0xec, 0x26, 0xb3, 0x35, 0x3b, 0x10, 0xa9, 0x03, 0xa6, 0xd0, 0xab, 0x1c, 0x4c};
   uint8_t u_out[32];
   x22519(k, u, u_out);
   // c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552
   for (int i = 0; i < 32; i++) 
     {
	printf("%02x", u_out[i]);
     }
   printf("\n");
   
   return 0;
}
