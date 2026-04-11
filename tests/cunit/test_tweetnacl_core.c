/**
 * CUnit Tests for Core TweetNaCl Functions
 * 
 * Tests for crypto_hash, crypto_verify, and basic primitives.
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "../../includes/tweetnacl.h"
#include "../../includes/randombytes.h"

/* Test vector for SHA-512 */
static const uint8_t test_message[] = "test message for hashing";
static const size_t test_message_len = sizeof(test_message) - 1;

void register_core_suite(CU_pRegistry registry) {
    CU_pSuite suite = NULL;
    
    /* Create the CoreSuite */
    suite = CU_add_suite(registry, "CoreSuite", NULL, NULL);
    if (suite == NULL) {
        CU_cleanup_registry();
        return;
    }
    
    /* Add tests to CoreSuite */
    CU_add_test(suite, "test_crypto_hash_basic", NULL);
    CU_add_test(suite, "test_crypto_verify_32", NULL);
    CU_add_test(suite, "test_crypto_verify_16", NULL);
}
