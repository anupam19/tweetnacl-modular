/*
 * TweetNaCl Core Cryptographic Tests (CUnit)
 * Tests for hash, verify, scalarmult primitives
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

/* Test fixtures */
static int core_setup(void) {
    return 0;
}

static int core_teardown(void) {
    return 0;
}

/* Test: SHA-512 hash is deterministic */
static void test_hash_deterministic(void) {
    const char* message = "Hello, TweetNaCl!";
    uint8_t hash1[64], hash2[64];
    
    CU_ASSERT(0 == crypto_hash(hash1, (const uint8_t*)message, strlen(message)));
    CU_ASSERT(0 == crypto_hash(hash2, (const uint8_t*)message, strlen(message)));
    CU_ASSERT(0 == memcmp(hash1, hash2, 64));
}

/* Test: Different messages produce different hashes */
static void test_hash_different(void) {
    const char* msg1 = "Message 1";
    const char* msg2 = "Message 2";
    uint8_t hash1[64], hash2[64];
    
    CU_ASSERT(0 == crypto_hash(hash1, (const uint8_t*)msg1, strlen(msg1)));
    CU_ASSERT(0 == crypto_hash(hash2, (const uint8_t*)msg2, strlen(msg2)));
    CU_ASSERT(0 != memcmp(hash1, hash2, 64));
}

/* Test: Empty message hash */
static void test_hash_empty(void) {
    uint8_t hash[64];
    CU_ASSERT(0 == crypto_hash(hash, NULL, 0));
}

/* Test: Scalar multiplication base point */
static void test_scalarmult_base(void) {
    uint8_t scalar[32];
    uint8_t result1[32], result2[32];
    
    randombytes(scalar, 32);
    
    CU_ASSERT(0 == crypto_scalarmult_base(result1, scalar));
    CU_ASSERT(0 == crypto_scalarmult_base(result2, scalar));
    CU_ASSERT(0 == memcmp(result1, result2, 32));
}

/* Test: Scalar multiplication with NULL input */
static void test_scalarmult_null(void) {
    uint8_t scalar[32];
    uint8_t result[32];
    
    randombytes(scalar, 32);
    
    /* Test with NULL output - should handle gracefully */
    CU_ASSERT(-1 == crypto_scalarmult_base(NULL, scalar));
}

/* Test: Verify function with equal buffers */
static void test_verify_equal(void) {
    uint8_t a[32], b[32];
    memset(a, 0x42, sizeof(a));
    memcpy(b, a, sizeof(b));
    
    CU_ASSERT(0 == crypto_verify_32(a, b));
}

/* Test: Verify function with different buffers */
static void test_verify_different(void) {
    uint8_t a[32], b[32];
    memset(a, 0x42, sizeof(a));
    memset(b, 0x43, sizeof(b));
    
    CU_ASSERT(-1 == crypto_verify_32(a, b));
}

/* Test: Verify with NULL pointers */
static void test_verify_null(void) {
    uint8_t a[32];
    memset(a, 0x42, sizeof(a));
    
    CU_ASSERT(-1 == crypto_verify_32(NULL, a));
    CU_ASSERT(-1 == crypto_verify_32(a, NULL));
}

/* Test suite creation */
CU_ErrorCode create_core_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("CoreSuite", core_setup, core_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    /* Add tests to suite */
    CU_add_test(suite, "test_hash_deterministic", test_hash_deterministic);
    CU_add_test(suite, "test_hash_different", test_hash_different);
    CU_add_test(suite, "test_hash_empty", test_hash_empty);
    CU_add_test(suite, "test_scalarmult_base", test_scalarmult_base);
    CU_add_test(suite, "test_scalarmult_null", test_scalarmult_null);
    CU_add_test(suite, "test_verify_equal", test_verify_equal);
    CU_add_test(suite, "test_verify_different", test_verify_different);
    CU_add_test(suite, "test_verify_null", test_verify_null);
    
    return CUE_SUCCESS;
}
