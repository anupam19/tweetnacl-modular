/*
 * CUnit Tests: crypto_box (Curve25519-XSalsa20-Poly1305)
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include <stdio.h>
#include "../includes/tweetnacl.h"
#include "../includes/randombytes.h"

static void test_box_keypair_nonzero(void) {
    uint8_t pk[32], sk[32];
    CU_ASSERT_EQUAL(crypto_box_keypair(pk, sk), 0);
    /* Public key should not be all zeros */
    int all_zero = 1;
    for (int i = 0; i < 32; i++) {
        if (pk[i] != 0) { all_zero = 0; break; }
    }
    CU_ASSERT_FALSE(all_zero);
}

static void test_box_encrypt_decrypt(void) {
    uint8_t pk1[32], sk1[32], pk2[32], sk2[32];
    uint8_t nonce[24];
    uint8_t msg[100], ct[100], pt[100];

    CU_ASSERT_EQUAL(crypto_box_keypair(pk1, sk1), 0);
    CU_ASSERT_EQUAL(crypto_box_keypair(pk2, sk2), 0);
    randombytes(nonce, 24);

    memset(msg, 0, 32);  /* Zero padding required by TweetNaCl */
    memcpy(msg + 32, "Hello crypto_box!", 17);

    CU_ASSERT_EQUAL(crypto_box(ct, msg, 100, nonce, pk2, sk1), 0);
    CU_ASSERT_EQUAL(crypto_box_open(pt, ct, 100, nonce, pk1, sk2), 0);
    CU_ASSERT_TRUE(memcmp(msg + 32, pt + 32, 17) == 0);
}

static void test_box_wrong_key_fails(void) {
    uint8_t pk1[32], sk1[32], pk2[32], sk2[32], pk3[32], sk3[32];
    uint8_t nonce[24];
    uint8_t msg[100], ct[100], pt[100];

    CU_ASSERT_EQUAL(crypto_box_keypair(pk1, sk1), 0);
    CU_ASSERT_EQUAL(crypto_box_keypair(pk2, sk2), 0);
    CU_ASSERT_EQUAL(crypto_box_keypair(pk3, sk3), 0);
    randombytes(nonce, 24);

    memset(msg, 0, 32);
    CU_ASSERT_EQUAL(crypto_box(ct, msg, 100, nonce, pk2, sk1), 0);

    /* Try to decrypt with wrong recipient key */
    CU_ASSERT_NOT_EQUAL(crypto_box_open(pt, ct, 100, nonce, pk3, sk2), 0);
}

static void test_box_tampered_ciphertext(void) {
    uint8_t pk[32], sk[32];
    uint8_t nonce[24];
    uint8_t msg[100], ct[100], pt[100];

    CU_ASSERT_EQUAL(crypto_box_keypair(pk, sk), 0);
    randombytes(nonce, 24);
    memset(msg, 0, 32);
    CU_ASSERT_EQUAL(crypto_box(ct, msg, 100, nonce, pk, sk), 0);

    /* Tamper with ciphertext */
    ct[40] ^= 0xFF;
    CU_ASSERT_NOT_EQUAL(crypto_box_open(pt, ct, 100, nonce, pk, sk), 0);
}

int init_box_tests(void) {
    CU_pSuite suite = CU_add_suite("BoxSuite", NULL, NULL);
    if (!suite) return CU_get_error();
    CU_ADD_TEST(suite, test_box_keypair_nonzero);
    CU_ADD_TEST(suite, test_box_encrypt_decrypt);
    CU_ADD_TEST(suite, test_box_wrong_key_fails);
    CU_ADD_TEST(suite, test_box_tampered_ciphertext);
    return CUE_SUCCESS;
}
