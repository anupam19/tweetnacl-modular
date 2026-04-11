/*
 * TweetNaCl Sign Tests (CUnit)
 * Tests for crypto_sign, crypto_sign_open, and keypair generation
 * 
 * Author: Anupam Datta <adbd04@gmail.com>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../../includes/tweetnacl.h"
#include "../../includes/randombytes.h"
#include "../../includes/secure_mem.h"

static int sign_setup(void) { return 0; }
static int sign_teardown(void) { return 0; }

/* Test: Basic signing and verification */
static void test_sign_basic(void) {
    uint8_t pk[32], sk[64];
    uint8_t message[50];
    uint8_t signed_msg[50 + 64];
    uint8_t verified_msg[50];
    unsigned long long signed_len, verified_len;
    
    CU_ASSERT(0 == crypto_sign_keypair(pk, sk));
    randombytes(message, sizeof(message));
    
    CU_ASSERT(0 == crypto_sign(signed_msg, &signed_len, message, sizeof(message), sk));
    CU_ASSERT(signed_len == sizeof(message) + 64);
    
    CU_ASSERT(0 == crypto_sign_open(verified_msg, &verified_len, signed_msg, signed_len, pk));
    CU_ASSERT(verified_len == sizeof(message));
    CU_ASSERT(0 == memcmp(message, verified_msg, sizeof(message)));
}

/* Test: Tampered signature detection */
static void test_sign_tampered(void) {
    uint8_t pk[32], sk[64];
    uint8_t message[50];
    uint8_t signed_msg[50 + 64];
    uint8_t verified_msg[50];
    unsigned long long signed_len, verified_len;
    
    CU_ASSERT(0 == crypto_sign_keypair(pk, sk));
    randombytes(message, sizeof(message));
    CU_ASSERT(0 == crypto_sign(signed_msg, &signed_len, message, sizeof(message), sk));
    
    /* Tamper with signature */
    signed_msg[10] ^= 0xFF;
    
    CU_ASSERT(-1 == crypto_sign_open(verified_msg, &verified_len, signed_msg, signed_len, pk));
}

/* Test: Signing with different messages */
static void test_sign_different_messages(void) {
    uint8_t pk[32], sk[64];
    uint8_t msg1[] = "Message 1";
    uint8_t msg2[] = "Message 2";
    uint8_t signed1[100], signed2[100];
    unsigned long long len1, len2;
    
    CU_ASSERT(0 == crypto_sign_keypair(pk, sk));
    CU_ASSERT(0 == crypto_sign(signed1, &len1, msg1, sizeof(msg1), sk));
    CU_ASSERT(0 == crypto_sign(signed2, &len2, msg2, sizeof(msg2), sk));
    
    /* Signatures should be different */
    CU_ASSERT(0 != memcmp(signed1, signed2, len1 < len2 ? len1 : len2));
}

/* Test: Empty message signing */
static void test_sign_empty(void) {
    uint8_t pk[32], sk[64];
    uint8_t message[1];
    uint8_t signed_msg[65];
    unsigned long long signed_len;
    
    CU_ASSERT(0 == crypto_sign_keypair(pk, sk));
    CU_ASSERT(0 == crypto_sign(signed_msg, &signed_len, message, 0, sk));
    CU_ASSERT(signed_len == 64);
}

/* Test: Key pair uniqueness */
static void test_keypair_unique(void) {
    uint8_t pk1[32], sk1[64];
    uint8_t pk2[32], sk2[64];
    
    CU_ASSERT(0 == crypto_sign_keypair(pk1, sk1));
    CU_ASSERT(0 == crypto_sign_keypair(pk2, sk2));
    
    /* Keys should be different */
    CU_ASSERT(0 != memcmp(pk1, pk2, 32));
    CU_ASSERT(0 != memcmp(sk1, sk2, 64));
}

CU_ErrorCode create_sign_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("SignSuite", sign_setup, sign_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    CU_add_test(suite, "test_sign_basic", test_sign_basic);
    CU_add_test(suite, "test_sign_tampered", test_sign_tampered);
    CU_add_test(suite, "test_sign_different_messages", test_sign_different_messages);
    CU_add_test(suite, "test_sign_empty", test_sign_empty);
    CU_add_test(suite, "test_keypair_unique", test_keypair_unique);
    
    return CUE_SUCCESS;
}
