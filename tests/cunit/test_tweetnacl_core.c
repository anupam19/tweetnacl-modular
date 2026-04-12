/*
 * CUnit Tests: TweetNaCl Core Primitives
 * Tests for SHA-512, verification, scalarmult
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include <stdio.h>
#include "../includes/tweetnacl.h"
#include "../includes/randombytes.h"
#include "vectors/test_vectors.h"

/* ─── Core Suite ──────────────────────────────────────────────────────── */

static void test_sha512_known_vectors(void) {
    uint8_t hash[64];

    /* SHA-512("abc") */
    crypto_hash(hash, (const uint8_t *)"abc", 3);
    CU_ASSERT_TRUE_FATAL(memcmp(hash, tv_sha512_abc, 64) == 0);

    /* SHA-512("") */
    crypto_hash(hash, (const uint8_t *)"", 0);
    CU_ASSERT_TRUE_FATAL(memcmp(hash, tv_sha512_empty, 64) == 0);
}

static void test_sha512_deterministic(void) {
    uint8_t hash1[64], hash2[64];
    const uint8_t msg[] = "deterministic test message";

    crypto_hash(hash1, msg, sizeof(msg));
    crypto_hash(hash2, msg, sizeof(msg));
    CU_ASSERT_TRUE_FATAL(memcmp(hash1, hash2, 64) == 0);
}

static void test_sha512_avalanche(void) {
    uint8_t hash1[64], hash2[64];
    const uint8_t msg1[] = "The quick brown fox jumps over the lazy dog";
    const uint8_t msg2[] = "The quick brown fox jumps over the lazy cog";

    crypto_hash(hash1, msg1, sizeof(msg1));
    crypto_hash(hash2, msg2, sizeof(msg2));

    /* At least half the bits should differ (avalanche effect) */
    int diff_bits = 0;
    for (int i = 0; i < 64; i++) {
        uint8_t xor = hash1[i] ^ hash2[i];
        for (int b = 0; b < 8; b++) {
            if (xor & (1 << b)) diff_bits++;
        }
    }
    CU_ASSERT_TRUE(diff_bits >= 256);
}

static void test_verify_16_equal(void) {
    uint8_t a[16], b[16];
    memset(a, 0x42, 16);
    memcpy(b, a, 16);
    CU_ASSERT_EQUAL(crypto_verify_16(a, b), 0);
}

static void test_verify_16_different(void) {
    uint8_t a[16], b[16];
    memset(a, 0x42, 16);
    memset(b, 0x42, 16);
    b[8] = 0xFF;
    CU_ASSERT_NOT_EQUAL(crypto_verify_16(a, b), 0);
}

static void test_verify_32_equal(void) {
    uint8_t a[32], b[32];
    memset(a, 0x55, 32);
    memcpy(b, a, 32);
    CU_ASSERT_EQUAL(crypto_verify_32(a, b), 0);
}

static void test_verify_32_different(void) {
    uint8_t a[32], b[32];
    memset(a, 0x55, 32);
    memset(b, 0x55, 32);
    b[31] ^= 0x01;
    CU_ASSERT_NOT_EQUAL(crypto_verify_32(a, b), 0);
}

static void test_scalarmult_base_known(void) {
    uint8_t result[32];
    crypto_scalarmult_base(result, tv_curve25519_base_scalar);
    CU_ASSERT_TRUE_FATAL(memcmp(result, tv_curve25519_base_result, 32) == 0);
}

static void test_scalarmult_base_deterministic(void) {
    uint8_t scalar[32], q1[32], q2[32];
    randombytes(scalar, 32);
    CU_ASSERT_EQUAL(crypto_scalarmult_base(q1, scalar), 0);
    CU_ASSERT_EQUAL(crypto_scalarmult_base(q2, scalar), 0);
    CU_ASSERT_TRUE(memcmp(q1, q2, 32) == 0);
}

int init_core_tests(void) {
    CU_pSuite suite = CU_add_suite("CoreSuite", NULL, NULL);
    if (!suite) return CU_get_error();

    CU_ADD_TEST(suite, test_sha512_known_vectors);
    CU_ADD_TEST(suite, test_sha512_deterministic);
    CU_ADD_TEST(suite, test_sha512_avalanche);
    CU_ADD_TEST(suite, test_verify_16_equal);
    CU_ADD_TEST(suite, test_verify_16_different);
    CU_ADD_TEST(suite, test_verify_32_equal);
    CU_ADD_TEST(suite, test_verify_32_different);
    CU_ADD_TEST(suite, test_scalarmult_base_known);
    CU_ADD_TEST(suite, test_scalarmult_base_deterministic);

    return CUE_SUCCESS;
}
