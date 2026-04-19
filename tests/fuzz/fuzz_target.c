/*
 * OSS-Fuzz Fuzz Target for TweetNaCl-Modular
 *
 * Build with:
 *   $CC -c -Iincludes -Iarch -Ipqc -fsanitize=fuzzer \
 *         src/*.c pqc/pqc.c tests/fuzz/fuzz_target.c \
 *         -o fuzz_tweetnacl
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../includes/tweetnacl.h"
#include "../includes/randombytes.h"

/* Deterministic PRNG seeded from fuzz data for reproducible tests */
static void seeded_randombytes_safe(uint8_t *buf, size_t len, const uint8_t *seed, size_t seed_len) {
    for (size_t i = 0; i < len; i++) {
        buf[i] = seed[i % seed_len];
    }
}

extern int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 128)
        return 0; /* Need minimum data */

    const uint8_t *pk = data;
    const uint8_t *sk = data + 32;
    const uint8_t *nonce = data + 64;
    const uint8_t *msg = data + 88;
    size_t msg_len = size - 88;

    /* Fuzz crypto_box_open with arbitrary inputs */
    if (msg_len >= 32) {
        uint8_t pt[4096];
        crypto_box_open(pt, msg, msg_len, nonce, pk, sk);
    }

    /* Fuzz crypto_sign_open */
    if (msg_len >= 64 && msg_len <= 4096) {
        uint8_t vm[4096];
        uint64_t vmlen = 0;
        crypto_sign_open(vm, &vmlen, msg, msg_len, pk);
    }

    /* Fuzz crypto_secretbox_open */
    if (msg_len >= 32) {
        uint8_t key[32];
        memcpy(key, data, 32);
        uint8_t pt[4096];
        crypto_secretbox_open(pt, msg, msg_len, nonce, key);
    }

    /* Fuzz crypto_hash with arbitrary length */
    uint8_t hash[64];
    crypto_hash(hash, msg, msg_len);

    /* Fuzz crypto_onetimeauth_verify */
    if (msg_len >= 16) {
        uint8_t key[32];
        memcpy(key, data, 32);
        crypto_onetimeauth_verify(msg, msg + 16, msg_len - 16, key);
    }

    return 0;
}
