/*
 * CUnit Tests: crypto_secretbox (XSalsa20-Poly1305)
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "../includes/tweetnacl.h"
#include "../includes/randombytes.h"

static void test_secretbox_roundtrip(void) {
    uint8_t key[32], nonce[24];
    uint8_t msg[100], ct[100], pt[100];

    randombytes_safe(key, 32);
    randombytes_safe(nonce, 24);
    memset(msg, 0, 32);
    memcpy(msg + 32, "secret message", 15);

    CU_ASSERT_EQUAL(crypto_secretbox(ct, msg, 100, nonce, key), 0);
    CU_ASSERT_EQUAL(crypto_secretbox_open(pt, ct, 100, nonce, key), 0);
    CU_ASSERT_TRUE(memcmp(msg + 32, pt + 32, 15) == 0);
}

static void test_secretbox_wrong_key(void) {
    uint8_t key1[32], key2[32], nonce[24];
    uint8_t msg[100], ct[100], pt[100];

    randombytes_safe(key1, 32);
    randombytes_safe(key2, 32);
    key2[0] ^= 0x01;
    randombytes_safe(nonce, 24);
    memset(msg, 0, 32);

    CU_ASSERT_EQUAL(crypto_secretbox(ct, msg, 100, nonce, key1), 0);
    CU_ASSERT_NOT_EQUAL(crypto_secretbox_open(pt, ct, 100, nonce, key2), 0);
}

static void test_secretbox_tampered(void) {
    uint8_t key[32], nonce[24];
    uint8_t msg[100], ct[100], pt[100];

    randombytes_safe(key, 32);
    randombytes_safe(nonce, 24);
    memset(msg, 0, 32);
    CU_ASSERT_EQUAL(crypto_secretbox(ct, msg, 100, nonce, key), 0);

    ct[50] ^= 0xFF;
    CU_ASSERT_NOT_EQUAL(crypto_secretbox_open(pt, ct, 100, nonce, key), 0);
}

int init_secretbox_tests(void) {
    CU_pSuite suite = CU_add_suite("SecretBoxSuite", NULL, NULL);
    if (!suite) return CU_get_error();
    CU_ADD_TEST(suite, test_secretbox_roundtrip);
    CU_ADD_TEST(suite, test_secretbox_wrong_key);
    CU_ADD_TEST(suite, test_secretbox_tampered);
    return CUE_SUCCESS;
}
