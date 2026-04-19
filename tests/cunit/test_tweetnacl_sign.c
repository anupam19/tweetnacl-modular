/*
 * CUnit Tests: crypto_sign (Ed25519)
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "../includes/tweetnacl.h"
#include "../includes/randombytes.h"

static void test_sign_keypair(void) {
    uint8_t pk[32], sk[64];
    CU_ASSERT_EQUAL(crypto_sign_keypair(pk, sk), 0);
    int all_zero = 1;
    for (int i = 0; i < 32; i++) { if (pk[i] != 0) { all_zero = 0; break; } }
    CU_ASSERT_FALSE(all_zero);
}

static void test_sign_roundtrip(void) {
    uint8_t pk[32], sk[64];
    uint8_t msg[50], sm[114], vm[114];
    u64 smlen, vmlen;

    CU_ASSERT_EQUAL(crypto_sign_keypair(pk, sk), 0);
    randombytes_safe(msg, sizeof(msg));
    CU_ASSERT_EQUAL(crypto_sign(sm, &smlen, msg, sizeof(msg), sk), 0);
    CU_ASSERT_EQUAL(smlen, sizeof(msg) + 64);
    CU_ASSERT_EQUAL(crypto_sign_open(vm, &vmlen, sm, smlen, pk), 0);
    CU_ASSERT_EQUAL(vmlen, sizeof(msg));
    CU_ASSERT_TRUE(memcmp(msg, vm, sizeof(msg)) == 0);
}

static void test_sign_tampered_signature(void) {
    uint8_t pk[32], sk[64];
    uint8_t msg[50], sm[114], vm[114];
    u64 smlen, vmlen;

    CU_ASSERT_EQUAL(crypto_sign_keypair(pk, sk), 0);
    randombytes_safe(msg, sizeof(msg));
    CU_ASSERT_EQUAL(crypto_sign(sm, &smlen, msg, sizeof(msg), sk), 0);

    sm[10] ^= 0xFF;
    CU_ASSERT_NOT_EQUAL(crypto_sign_open(vm, &vmlen, sm, smlen, pk), 0);
}

static void test_sign_tampered_message(void) {
    uint8_t pk[32], sk[64];
    uint8_t msg[50], sm[114], vm[114];
    u64 smlen, vmlen;

    CU_ASSERT_EQUAL(crypto_sign_keypair(pk, sk), 0);
    randombytes_safe(msg, sizeof(msg));
    CU_ASSERT_EQUAL(crypto_sign(sm, &smlen, msg, sizeof(msg), sk), 0);

    /* Tamper with the message part of signed message */
    sm[70] ^= 0x01;
    CU_ASSERT_NOT_EQUAL(crypto_sign_open(vm, &vmlen, sm, smlen, pk), 0);
}

int init_sign_tests(void) {
    CU_pSuite suite = CU_add_suite("SignSuite", NULL, NULL);
    if (!suite)
        return CU_get_error();
    CU_ADD_TEST(suite, test_sign_keypair);
    CU_ADD_TEST(suite, test_sign_roundtrip);
    CU_ADD_TEST(suite, test_sign_tampered_signature);
    CU_ADD_TEST(suite, test_sign_tampered_message);
    return CUE_SUCCESS;
}
