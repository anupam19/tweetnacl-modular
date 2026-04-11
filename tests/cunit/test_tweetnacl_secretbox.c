/*
 * TweetNaCl SecretBox Tests (CUnit)
 * Tests for crypto_secretbox and crypto_secretbox_open
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

static int secretbox_setup(void) { return 0; }
static int secretbox_teardown(void) { return 0; }

/* Test: Basic secretbox encryption/decryption */
static void test_secretbox_basic(void) {
    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    randombytes(key, 32);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memset(message + 32, 'A', sizeof(message) - 32);
    
    CU_ASSERT(0 == crypto_secretbox(ciphertext, message, sizeof(message), nonce, key));
    CU_ASSERT(0 == crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, key));
    CU_ASSERT(0 == memcmp(message + 32, decrypted + 32, sizeof(message) - 32));
}

/* Test: Secretbox with wrong key fails */
static void test_secretbox_wrong_key(void) {
    uint8_t key1[32], key2[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    randombytes(key1, 32);
    randombytes(key2, 32);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memcpy(message + 32, "Test", 5);
    
    CU_ASSERT(0 == crypto_secretbox(ciphertext, message, sizeof(message), nonce, key1));
    CU_ASSERT(-1 == crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, key2));
}

/* Test: Secretbox with tampered ciphertext */
static void test_secretbox_tampered(void) {
    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    randombytes(key, 32);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memcpy(message + 32, "Tamper", 7);
    
    CU_ASSERT(0 == crypto_secretbox(ciphertext, message, sizeof(message), nonce, key));
    
    /* Tamper with ciphertext */
    ciphertext[40] ^= 0xFF;
    
    CU_ASSERT(-1 == crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, key));
}

/* Test: Secretbox with zero-length payload */
static void test_secretbox_zero_length(void) {
    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t message[32];
    uint8_t ciphertext[32];
    uint8_t decrypted[32];
    
    randombytes(key, 32);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    
    CU_ASSERT(0 == crypto_secretbox(ciphertext, message, sizeof(message), nonce, key));
    CU_ASSERT(0 == crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, key));
}

/* Test: Nonce reuse produces same ciphertext */
static void test_secretbox_nonce_reuse(void) {
    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t message[64];
    uint8_t ciphertext1[64], ciphertext2[64];
    
    randombytes(key, 32);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memcpy(message + 32, "Same", 5);
    
    CU_ASSERT(0 == crypto_secretbox(ciphertext1, message, sizeof(message), nonce, key));
    CU_ASSERT(0 == crypto_secretbox(ciphertext2, message, sizeof(message), nonce, key));
    
    /* Same key+nonce+message should produce same ciphertext */
    CU_ASSERT(0 == memcmp(ciphertext1, ciphertext2, sizeof(ciphertext1)));
}

CU_ErrorCode create_secretbox_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("SecretBoxSuite", secretbox_setup, secretbox_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    CU_add_test(suite, "test_secretbox_basic", test_secretbox_basic);
    CU_add_test(suite, "test_secretbox_wrong_key", test_secretbox_wrong_key);
    CU_add_test(suite, "test_secretbox_tampered", test_secretbox_tampered);
    CU_add_test(suite, "test_secretbox_zero_length", test_secretbox_zero_length);
    CU_add_test(suite, "test_secretbox_nonce_reuse", test_secretbox_nonce_reuse);
    
    return CUE_SUCCESS;
}
