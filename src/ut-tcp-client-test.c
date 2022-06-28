#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut.h"

// https://datatracker.ietf.org/doc/html/rfc7748

static uint8_t one[32] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t zero[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t curve25519_a24[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t curve25519_p_minus_2[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void set(const uint8_t *in, uint8_t *out) {
  for (size_t i = 0; i < 32; i++) {
    out[i] = in[i];
  }
}

static void add(const uint8_t *a, const uint8_t *b, uint8_t *out) {
  for (size_t i = 0; i < 32; i++) {
    out[i] = a[i] + b[i];
  }
}

static void sub(const uint8_t *a, const uint8_t *b, uint8_t *out) {
  for (size_t i = 0; i < 32; i++) {
    out[i] = a[i] - b[i];
  }
}

static void mul(const uint8_t *a, const uint8_t *b, uint8_t *out) {}

static void pow256(const uint8_t *a, const uint8_t *b, uint8_t *out) {}

static void cswap(bool swap, uint8_t *x2, uint8_t *x3) {
  uint8_t mask = swap ? 0xff : 0x00;
  for (size_t i = 0; i < 32; i++) {
    uint8_t dummy = mask & (x2[i] ^ x3[i]);
    x2[i] = x2[i] ^ dummy;
    x3[i] = x3[i] ^ dummy;
  }
}

static void unknown(const uint8_t *k, const uint8_t *u, const uint8_t *a24,
                    const uint8_t *p_minus_2, uint8_t *out) {
  uint8_t x1[32], x2[32], z2[32], x3[32], z3[32];

  set(u, x1);
  set(one, x2);
  set(zero, z2);
  set(u, x3);
  set(zero, z3);

  uint8_t swap = 0;
  for (ssize_t i = 254; i >= 0; i--) {
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
    mul(a24, e, t);
    add(aa, t, t2);
    mul(e, t2, z2);
  }

  cswap(swap, x2, x3);
  cswap(swap, z2, z3);

  // Return x_2 * (z_2^(p - 2))
  uint8_t t[32];
  pow256(z2, p_minus_2, t);
  mul(x2, t, out);
}

static void x22519(const uint8_t *k, const uint8_t *u, uint8_t *out) {
  unknown(k, u, curve25519_a24, curve25519_p_minus_2, out);
}

static bool decode_tls_change_cipher_spec(UtObject *data) {
  printf("change_cipher_spec\n");
  return true;
}

static bool decode_tls_server_hello(UtObject *data) {
  size_t length = ut_list_get_length(data);
  size_t offset = 0;

  if (length < 2) {
    return false;
  }
  uint16_t version = ut_uint8_list_get_uint16_be(data, offset);
  assert(version == 0x0303); // TLS v1.2
  offset += 2;

  if (length < offset + 32) {
    return false;
  }
  // FIXME: 32 bytes of random data
  offset += 32;

  if (length < offset + 1) {
    return false;
  }
  uint16_t session_length = ut_uint8_list_get_element(data, offset);
  offset++;
  if (length < offset + session_length) {
    return false;
  }
  offset += session_length;

  if (length < offset + 2) {
    return false;
  }
  /*uint16_t cipher_suite = */ ut_uint8_list_get_uint16_be(data, offset);
  offset += 2;

  if (length < offset + 1) {
    return false;
  }
  /*uint8_t compression_method = */ ut_uint8_list_get_element(data, offset);
  offset++;

  if (length < offset + 2) {
    return false;
  }
  uint16_t extensions_length = ut_uint8_list_get_uint16_be(data, offset);
  offset += 2;

  if (length < offset + extensions_length) {
    return false;
  }

  printf("server_hello\n");
  return true;
}

static bool decode_tls_handshake_record(UtObject *data) {
  if (ut_list_get_length(data) < 4) {
    return false;
  }

  uint8_t type = ut_uint8_list_get_element(data, 0);
  uint32_t length = ut_uint8_list_get_element(data, 1) << 16 |
                    ut_uint8_list_get_element(data, 2) << 8 |
                    ut_uint8_list_get_element(data, 3);

  size_t total_length = length + 4;
  if (ut_list_get_length(data) < total_length) {
    return 0;
  }
  UtObjectRef payload = ut_list_get_sublist(data, 4, length);

  switch (type) {
  case 0x02:
    return decode_tls_server_hello(payload);
    break;
  default:
    printf("unknown handshake type %02x\n", type);
    return true;
  }
}

static void decode_tls_application_data(UtObject *data) {
  printf("application_data\n");
}

static size_t decode_tls_record(UtObject *data) {
  if (ut_list_get_length(data) < 5) {
    return 0;
  }

  uint8_t type = ut_uint8_list_get_element(data, 0);
  uint16_t version = ut_uint8_list_get_uint16_be(data, 1);
  assert(version == 0x0303); // TLS v1.2
  uint16_t length = ut_uint8_list_get_uint16_be(data, 3);

  size_t total_length = length + 5;
  if (ut_list_get_length(data) < total_length) {
    return 0;
  }
  UtObjectRef payload = ut_list_get_sublist(data, 5, length);

  switch (type) {
  case 0x14:
    decode_tls_change_cipher_spec(payload);
    break;
  case 0x16:
    decode_tls_handshake_record(payload);
    break;
  case 0x17:
    decode_tls_application_data(payload);
    break;
  default:
    printf("unknown tls record type %02x\n", type);
    break;
  }

  return total_length;
}

static size_t tls_read_cb(void *user_data, UtObject *data, bool complete) {
  // printf("tls read:\n%s'\n", ut_object_to_string(data));
  return decode_tls_record(data);
}

void append_uint24_be(UtObject *object, uint32_t value) {
  uint8_t data[3] = {(value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff};
  ut_uint8_list_append_block(object, data, 3);
}

static void add_extension(UtObject *object, uint16_t id, UtObject *data) {
  ut_uint8_list_append_uint16_be(object, id);
  assert(ut_list_get_length(data) <= 0xffff);
  ut_uint8_list_append_uint16_be(object, ut_list_get_length(data));
  ut_list_append_list(object, data);
}

static void tls_connect_cb(void *user_data) {
  UtObject *tcp_client = user_data;

  printf("tls connect\n");

  UtObjectRef random = ut_uint8_list_new_with_data(
      32, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
      0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
      0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f);

  UtObjectRef session_id = ut_uint8_list_new_with_data(
      32, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
      0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
      0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff);

  UtObjectRef cipher_suites = ut_uint8_list_new();
  ut_uint8_list_append_uint16_be(cipher_suites, 0x1302);
  ut_uint8_list_append_uint16_be(cipher_suites, 0x1303);
  ut_uint8_list_append_uint16_be(cipher_suites, 0x1301);
  ut_uint8_list_append_uint16_be(cipher_suites, 0x00ff);

  UtObjectRef compression_data = ut_uint8_list_new();
  ut_uint8_list_append(compression_data, 0); // no compression

  UtObjectRef extensions = ut_uint8_list_new();

  const char *hostname = "google.com";
  size_t hostname_length = strlen(hostname);
  UtObjectRef entry = ut_uint8_list_new();
  ut_uint8_list_append(entry, 0); // DNS hostname
  assert(hostname_length <= 0xffff);
  ut_uint8_list_append_uint16_be(entry, hostname_length);
  ut_uint8_list_append_block(entry, (uint8_t *)hostname, hostname_length);
  UtObjectRef server_name_extension = ut_uint8_list_new();
  assert(ut_list_get_length(entry) <= 0xffff);
  ut_uint8_list_append_uint16_be(server_name_extension,
                                 ut_list_get_length(entry));
  ut_list_append_list(server_name_extension, entry);
  add_extension(extensions, 0x0000, server_name_extension);

  UtObjectRef ec_point_formats = ut_uint8_list_new();
  ut_uint8_list_append(ec_point_formats, 0x00); // uncompressed
  ut_uint8_list_append(ec_point_formats, 0x01); // ansiX962_compressed_prime
  ut_uint8_list_append(ec_point_formats, 0x02); // ansiX962_compressed_char2
  UtObjectRef ec_point_formats_extension = ut_uint8_list_new();
  assert(ut_list_get_length(ec_point_formats) <= 0xff);
  ut_uint8_list_append(ec_point_formats_extension,
                       ut_list_get_length(ec_point_formats));
  ut_list_append_list(ec_point_formats_extension, ec_point_formats);
  add_extension(extensions, 0x000b, ec_point_formats_extension);

  UtObjectRef curves = ut_uint8_list_new();
  ut_uint8_list_append_uint16_be(curves, 0x001d);
  ut_uint8_list_append_uint16_be(curves, 0x0017);
  ut_uint8_list_append_uint16_be(curves, 0x001e);
  ut_uint8_list_append_uint16_be(curves, 0x0019);
  ut_uint8_list_append_uint16_be(curves, 0x0018);
  ut_uint8_list_append_uint16_be(curves, 0x0100);
  ut_uint8_list_append_uint16_be(curves, 0x0101);
  ut_uint8_list_append_uint16_be(curves, 0x0102);
  ut_uint8_list_append_uint16_be(curves, 0x0103);
  ut_uint8_list_append_uint16_be(curves, 0x0104);
  UtObjectRef supported_groups_extension = ut_uint8_list_new();
  assert(ut_list_get_length(curves) <= 0xffff);
  ut_uint8_list_append_uint16_be(supported_groups_extension,
                                 ut_list_get_length(curves));
  ut_list_append_list(supported_groups_extension, curves);
  add_extension(extensions, 0x000a, supported_groups_extension);

  UtObjectRef session_ticket_extension = ut_uint8_list_new();
  add_extension(extensions, 0x0023, session_ticket_extension);

  UtObjectRef encrypt_then_mac_extension = ut_uint8_list_new();
  add_extension(extensions, 0x0016, encrypt_then_mac_extension);

  UtObjectRef extended_master_secret_extension = ut_uint8_list_new();
  add_extension(extensions, 0x0017, extended_master_secret_extension);

  UtObjectRef signature_algorithms = ut_uint8_list_new();
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0403);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0503);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0603);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0807);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0808);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0809);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x080a);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x080b);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0804);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0805);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0806);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0401);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0501);
  ut_uint8_list_append_uint16_be(signature_algorithms, 0x0601);
  UtObjectRef signature_algorithms_extension = ut_uint8_list_new();
  assert(ut_list_get_length(signature_algorithms) <= 0xffff);
  ut_uint8_list_append_uint16_be(signature_algorithms_extension,
                                 ut_list_get_length(signature_algorithms));
  ut_list_append_list(signature_algorithms_extension, signature_algorithms);
  add_extension(extensions, 0x000d, signature_algorithms_extension);

  UtObjectRef supported_versions = ut_uint8_list_new();
  ut_uint8_list_append_uint16_be(supported_versions, 0x0304); // TLS v1.3
  UtObjectRef supported_versions_extension = ut_uint8_list_new();
  assert(ut_list_get_length(supported_versions) <= 0xff);
  ut_uint8_list_append(supported_versions_extension,
                       ut_list_get_length(supported_versions));
  ut_list_append_list(supported_versions_extension, supported_versions);
  add_extension(extensions, 0x002b, supported_versions_extension);

  UtObjectRef psk_key_exchange_modes = ut_uint8_list_new();
  ut_uint8_list_append(psk_key_exchange_modes,
                       0x01); // PSK with (EC)DHE key establishment
  UtObjectRef psk_key_exchange_modes_extension = ut_uint8_list_new();
  assert(ut_list_get_length(psk_key_exchange_modes) <= 0xff);
  ut_uint8_list_append(psk_key_exchange_modes_extension,
                       ut_list_get_length(psk_key_exchange_modes));
  ut_list_append_list(psk_key_exchange_modes_extension, psk_key_exchange_modes);
  add_extension(extensions, 0x002d, psk_key_exchange_modes_extension);

  UtObjectRef public_key = ut_uint8_list_new_with_data(
      32, 0x35, 0x80, 0x72, 0xd6, 0x36, 0x58, 0x80, 0xd1, 0xae, 0xea, 0x32,
      0x9a, 0xdf, 0x91, 0x21, 0x38, 0x38, 0x51, 0xed, 0x21, 0xa2, 0x8e, 0x3b,
      0x75, 0xe9, 0x65, 0xd0, 0xd2, 0xcd, 0x16, 0x62, 0x54);
  UtObjectRef key_share_data = ut_uint8_list_new();
  ut_uint8_list_append_uint16_be(key_share_data, 0x001d); // x25519
  assert(ut_list_get_length(public_key) <= 0xffff);
  ut_uint8_list_append_uint16_be(key_share_data,
                                 ut_list_get_length(public_key));
  ut_list_append_list(key_share_data, public_key);
  UtObjectRef key_share_extension = ut_uint8_list_new();
  assert(ut_list_get_length(key_share_data) <= 0xffff);
  ut_uint8_list_append_uint16_be(key_share_extension,
                                 ut_list_get_length(key_share_data));
  ut_list_append_list(key_share_extension, key_share_data);
  add_extension(extensions, 0x0033, key_share_extension);

  UtObjectRef hello = ut_uint8_list_new();
  ut_uint8_list_append_uint16_be(hello, 0x0303); // TLS v1.2
  assert(ut_list_get_length(random) == 32);
  ut_list_append_list(hello, random);
  assert(ut_list_get_length(session_id) <= 0xff);
  ut_uint8_list_append(hello, ut_list_get_length(session_id));
  ut_list_append_list(hello, session_id);
  assert(ut_list_get_length(cipher_suites) <= 0xffff);
  ut_uint8_list_append_uint16_be(hello, ut_list_get_length(cipher_suites));
  ut_list_append_list(hello, cipher_suites);
  assert(ut_list_get_length(compression_data) <= 0xff);
  ut_uint8_list_append(hello, ut_list_get_length(compression_data));
  ut_list_append_list(hello, compression_data);
  assert(ut_list_get_length(extensions) <= 0xffff);
  ut_uint8_list_append_uint16_be(hello, ut_list_get_length(extensions));
  ut_list_append_list(hello, extensions);

  UtObjectRef record = ut_uint8_list_new();
  ut_uint8_list_append(record, 1); // client_hello
  assert(ut_list_get_length(hello) <= 0xffffff);
  append_uint24_be(record, ut_list_get_length(hello));
  ut_list_append_list(record, hello);

  UtObjectRef request = ut_uint8_list_new();
  ut_uint8_list_append(request, 0x16);             // handshake record
  ut_uint8_list_append_uint16_be(request, 0x0301); // TLS v1.0
  assert(ut_list_get_length(record) <= 0xffff);
  ut_uint8_list_append_uint16_be(request, ut_list_get_length(record));
  ut_list_append_list(request, record);

  ut_output_stream_write(tcp_client, request);
  ut_input_stream_read(tcp_client, tls_read_cb, NULL, NULL);
}

int main(int argc, char **argv) {
  UtObjectRef tcp_client = ut_tcp_client_new("google.com", 443);
  ut_tcp_client_connect(tcp_client, tls_connect_cb, tcp_client, NULL);

  ut_event_loop_run();

  return 0;
}
