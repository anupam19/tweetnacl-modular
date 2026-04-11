/*
 * TweetNaCl Box Tests (CUnit)
 * Tests for crypto_box and crypto_box_open
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

static int box_setup(void) { return 0; }
static int box_teardown(void) { return 0; }

/* Test: Basic box encryption/decryption */
static void test_box_basic(void) {
    uint8_t pk1[32], sk1[32];
    uint8_t pk2[32], sk2[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    crypto_box_keypair(pk1, sk1);
    crypto_box_keypair(pk2, sk2);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memset(message + 32, 'B', sizeof(message) - 32);
    
    CU_ASSERT(0 == crypto_box(ciphertext, message, sizeof(message), nonce, pk2, sk1));
    CU_ASSERT(0 == crypto_box_open(decrypted, ciphertext, sizeof(message), nonce, pk1, sk2));
    CU_ASSERT(0 == memcmp(message + 32, decrypted + 32, sizeof(message) - 32));
}

/* Test: Box with wrong key fails */
static void test_box_wrong_key(void) {
    uint8_t pk1[32], sk1[32];
    uint8_t pk2[32], sk2[32];
    uint8_t pk3[32], sk3[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    crypto_box_keypair(pk1, sk1);
    crypto_box_keypair(pk2, sk2);
    crypto_box_keypair(pk3, sk3);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memcpy(message + 32, "Test", 5);
    
    CU_ASSERT(0 == crypto_box(ciphertext, message, sizeof(message), nonce, pk2, sk1));
    /* Decrypt with wrong key pair */
    CU_ASSERT(-1 == crypto_box_open(decrypted, ciphertext, sizeof(message), nonce, pk3, sk3));
}

/* Test: Box with tampered ciphertext fails */
static void test_box_tampered(void) {
    uint8_t pk[32], sk[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    crypto_box_keypair(pk, sk);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memcpy(message + 32, "Tamper test", 12);
    
    CU_ASSERT(0 == crypto_box(ciphertext, message, sizeof(message), nonce, pk, sk));
    
    /* Tamper with ciphertext */
    ciphertext[50] ^= 0xFF;
    
    CU_ASSERT(-1 == crypto_box_open(decrypted, ciphertext, sizeof(message), nonce, pk, sk));
}

/* Test: Box with zero-length message */
static void test_box_zero_length(void) {
    uint8_t pk[32], sk[32];
    uint8_t nonce[24];
    uint8_t message[32];
    uint8_t ciphertext[32];
    uint8_t decrypted[32];
    
    crypto_box_keypair(pk, sk);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    
    CU_ASSERT(0 == crypto_box(ciphertext, message, sizeof(message), nonce, pk, sk));
    CU_ASSERT(0 == crypto_box_open(decrypted, ciphertext, sizeof(message), nonce, pk, sk));
}

CU_ErrorCode create_box_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("BoxSuite", box_setup, box_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    CU_add_test(suite, "test_box_basic", test_box_basic);
    CU_add_test(suite, "test_box_wrong_key", test_box_wrong_key);
    CU_add_test(suite, "test_box_tampered", test_box_tampered);
    CU_add_test(suite, "test_box_zero_length", test_box_zero_length);
    
    return CUE_SUCCESS;
}
