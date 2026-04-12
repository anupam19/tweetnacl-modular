/*
 * TweetNaCl-Modular Benchmark Suite
 * Uses clock_gettime(CLOCK_MONOTONIC) for cycle-accurate timing
 */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../includes/tweetnacl.h"
#include "../includes/randombytes.h"
#include "../includes/secure_mem.h"

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

#define BENCH(name, ops, body) do { \
    double start = now_ms(); \
    for (int _i = 0; _i < (ops); _i++) { body; } \
    double elapsed = now_ms() - start; \
    printf("%-30s %8.3f ms  (%.0f ops/sec)\n", \
           name, elapsed, (ops) * 1000.0 / elapsed); \
} while(0)

int main(void) {
    printf("TweetNaCl-Modular Benchmarks\n");
    printf("============================\n\n");

    /* SHA-512 */
    {
        uint8_t hash[64], msg[1024];
        randombytes(msg, sizeof(msg));
        BENCH("SHA-512 (1KB)", 1000, crypto_hash(hash, msg, sizeof(msg)));
    }

    /* crypto_secretbox */
    {
        uint8_t key[32], nonce[24], msg[1024], ct[1024];
        randombytes(key, 32);
        randombytes(nonce, 24);
        memset(msg, 0, 32);
        randombytes(msg + 32, sizeof(msg) - 32);
        BENCH("SecretBox (1KB)", 1000,
            crypto_secretbox(ct, msg, sizeof(msg), nonce, key));
    }

    /* crypto_box */
    {
        uint8_t pk1[32], sk1[32], pk2[32], sk2[32], nonce[24];
        uint8_t msg[1024], ct[1024];
        crypto_box_keypair(pk1, sk1);
        crypto_box_keypair(pk2, sk2);
        randombytes(nonce, 24);
        memset(msg, 0, 32);
        BENCH("Box (1KB)", 100,
            crypto_box(ct, msg, sizeof(msg), nonce, pk2, sk1));
    }

    /* crypto_sign */
    {
        uint8_t pk[32], sk[64];
        uint8_t msg[512], sm[512 + 64];
        uint64_t smlen;
        crypto_sign_keypair(pk, sk);
        randombytes(msg, sizeof(msg));
        BENCH("Ed25519 Sign (512B)", 50,
            crypto_sign(sm, &smlen, msg, sizeof(msg), sk));
    }

    /* crypto_sign_open */
    {
        uint8_t pk[32], sk[64];
        uint8_t msg[512], sm[512 + 64], vm[512 + 64];
        uint64_t smlen, vmlen;
        crypto_sign_keypair(pk, sk);
        randombytes(msg, sizeof(msg));
        crypto_sign(sm, &smlen, msg, sizeof(msg), sk);
        BENCH("Ed25519 Verify (512B)", 50,
            crypto_sign_open(vm, &vmlen, sm, smlen, pk));
    }

    /* crypto_scalarmult_base */
    {
        uint8_t scalar[32], result[32];
        randombytes(scalar, 32);
        BENCH("ScalarMult Base", 100,
            crypto_scalarmult_base(result, scalar));
    }

    /* secure_zero */
    {
        uint8_t buf[1024];
        memset(buf, 0xAA, sizeof(buf));
        BENCH("Secure Zero (1KB)", 10000,
            secure_zero(buf, sizeof(buf)));
    }

    printf("\n");
    return 0;
}
