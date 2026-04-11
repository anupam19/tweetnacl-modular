/*
 * TweetNaCl PQC Tests (CUnit)
 * Tests for Post-Quantum Cryptography interface functions
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
#include "../../pqc/pqc.h"

static int pqc_setup(void) { return 0; }
static int pqc_teardown(void) { return 0; }

/* Test: PQC get parameters */
static void test_pqc_get_params(void) {
    pqc_params_t params;
    pqc_result_t result;
    
    result = pqc_get_params(PQC_KYBER768, &params);
    CU_ASSERT(PQC_SUCCESS == result);
    CU_ASSERT(params.public_key_size > 0);
    CU_ASSERT(params.secret_key_size > 0);
    CU_ASSERT(params.ciphertext_size > 0);
}

/* Test: PQC key generation */
static void test_pqc_keygen(void) {
    uint8_t pk[2000], sk[3000];
    pqc_result_t result;
    
    result = pqc_keygen(PQC_KYBER512, pk, sizeof(pk), sk, sizeof(sk), NULL, 0);
    CU_ASSERT(PQC_SUCCESS == result);
}

/* Test: PQC encapsulation/decapsulation */
static void test_pqc_encapsulate_decapsulate(void) {
    uint8_t pk[2000], sk[3000];
    uint8_t ct[1500], ss[32], ss2[32];
    size_t ct_len;
    pqc_result_t result;
    pqc_params_t params;
    
    /* Generate key pair */
    result = pqc_keygen(PQC_KYBER512, pk, sizeof(pk), sk, sizeof(sk), NULL, 0);
    CU_ASSERT(PQC_SUCCESS == result);
    
    /* Get params for sizes */
    result = pqc_get_params(PQC_KYBER512, &params);
    CU_ASSERT(PQC_SUCCESS == result);
    
    /* Encapsulate */
    ct_len = sizeof(ct);
    result = pqc_encapsulate(PQC_KYBER512, pk, params.public_key_size, 
                            ct, &ct_len, ss, sizeof(ss), NULL, 0);
    CU_ASSERT(PQC_SUCCESS == result);
    
    /* Decapsulate */
    result = pqc_decapsulate(PQC_KYBER512, sk, params.secret_key_size, 
                            ct, ct_len, ss2, sizeof(ss2));
    CU_ASSERT(PQC_SUCCESS == result);
    
    /* Shared secrets should match */
    CU_ASSERT(0 == memcmp(ss, ss2, 32));
}

/* Test: PQC hybrid keygen */
static void test_pqc_hybrid_keygen(void) {
    uint8_t hybrid_pk[2000], hybrid_sk[2500];
    size_t hybrid_pk_len, hybrid_sk_len;
    pqc_result_t result;
    
    result = pqc_hybrid_keygen(PQC_KYBER768, 
                              hybrid_pk, &hybrid_pk_len,
                              hybrid_sk, &hybrid_sk_len);
    CU_ASSERT(PQC_SUCCESS == result);
    CU_ASSERT(hybrid_pk_len > 1000);
    CU_ASSERT(hybrid_sk_len > 1000);
}

/* Test: PQC hybrid encapsulate/decapsulate */
static void test_pqc_hybrid_encapsulate(void) {
    uint8_t hybrid_pk[2000], hybrid_sk[2500];
    uint8_t hybrid_ct[2000], hybrid_ss[32], hybrid_ss2[32];
    size_t hybrid_pk_len, hybrid_sk_len, hybrid_ct_len;
    pqc_result_t result;
    
    /* Generate hybrid keys */
    result = pqc_hybrid_keygen(PQC_KYBER768, 
                              hybrid_pk, &hybrid_pk_len,
                              hybrid_sk, &hybrid_sk_len);
    CU_ASSERT(PQC_SUCCESS == result);
    
    /* Encapsulate */
    hybrid_ct_len = sizeof(hybrid_ct);
    result = pqc_hybrid_encapsulate(hybrid_pk, hybrid_pk_len,
                                   hybrid_ct, &hybrid_ct_len,
                                   hybrid_ss, sizeof(hybrid_ss));
    CU_ASSERT(PQC_SUCCESS == result);
    
    /* Decapsulate */
    result = pqc_hybrid_decapsulate(hybrid_sk, hybrid_sk_len,
                                   hybrid_ct, hybrid_ct_len,
                                   hybrid_ss2, sizeof(hybrid_ss2));
    CU_ASSERT(PQC_SUCCESS == result);
    
    /* Shared secrets should match */
    CU_ASSERT(0 == memcmp(hybrid_ss, hybrid_ss2, 32));
}

/* Test: PQC error handling - invalid algorithm */
static void test_pqc_invalid_algorithm(void) {
    pqc_params_t params;
    pqc_result_t result;
    
    result = pqc_get_params((pqc_algorithm_t)999, &params);
    CU_ASSERT(PQC_ERROR_ALGORITHM_NOT_SUPPORTED == result);
}

/* Test: PQC error handling - NULL parameters */
static void test_pqc_null_params(void) {
    uint8_t pk[100], sk[100];
    pqc_result_t result;
    
    result = pqc_keygen(PQC_KYBER512, NULL, sizeof(pk), sk, sizeof(sk), NULL, 0);
    CU_ASSERT(PQC_ERROR_INVALID_PARAM == result);
    
    result = pqc_keygen(PQC_KYBER512, pk, sizeof(pk), NULL, sizeof(sk), NULL, 0);
    CU_ASSERT(PQC_ERROR_INVALID_PARAM == result);
}

/* Test: PQC error handling - buffer too small */
static void test_pqc_buffer_too_small(void) {
    uint8_t pk[100], sk[100];
    pqc_result_t result;
    
    result = pqc_keygen(PQC_KYBER768, pk, 100, sk, 100, NULL, 0);
    CU_ASSERT(PQC_ERROR_BUFFER_TOO_SMALL == result);
}

/* Test: PQC result to string conversion */
static void test_pqc_result_to_string(void) {
    const char* str;
    
    str = pqc_result_to_string(PQC_SUCCESS);
    CU_ASSERT_STRING_EQUAL("Success", str);
    
    str = pqc_result_to_string(PQC_ERROR_INVALID_PARAM);
    CU_ASSERT_STRING_EQUAL("Invalid parameter", str);
}

CU_ErrorCode create_pqc_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("PqCSuite", pqc_setup, pqc_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    CU_add_test(suite, "test_pqc_get_params", test_pqc_get_params);
    CU_add_test(suite, "test_pqc_keygen", test_pqc_keygen);
    CU_add_test(suite, "test_pqc_encapsulate_decapsulate", test_pqc_encapsulate_decapsulate);
    CU_add_test(suite, "test_pqc_hybrid_keygen", test_pqc_hybrid_keygen);
    CU_add_test(suite, "test_pqc_hybrid_encapsulate", test_pqc_hybrid_encapsulate);
    CU_add_test(suite, "test_pqc_invalid_algorithm", test_pqc_invalid_algorithm);
    CU_add_test(suite, "test_pqc_null_params", test_pqc_null_params);
    CU_add_test(suite, "test_pqc_buffer_too_small", test_pqc_buffer_too_small);
    CU_add_test(suite, "test_pqc_result_to_string", test_pqc_result_to_string);
    
    return CUE_SUCCESS;
}
