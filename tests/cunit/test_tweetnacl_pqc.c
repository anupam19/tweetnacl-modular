/*
 * CUnit Tests: PQC Interface
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "../pqc/pqc.h"

static void test_pqc_kyber768_params(void) {
    pqc_params_t params;
    CU_ASSERT_EQUAL(pqc_get_params(PQC_KYBER768, &params), PQC_SUCCESS);
    CU_ASSERT(params.public_key_size > 0);
    CU_ASSERT(params.secret_key_size > 0);
}

static void test_pqc_kyber_roundtrip(void) {
    pqc_params_t params;
    CU_ASSERT_EQUAL(pqc_get_params(PQC_KYBER512, &params), PQC_SUCCESS);

    uint8_t pk[2000], sk[3000];
    CU_ASSERT_EQUAL(pqc_keygen(PQC_KYBER512, pk, sizeof(pk), sk, sizeof(sk), NULL, 0), PQC_SUCCESS);

    uint8_t ct[1000], ss[32], ss2[32];
    size_t ct_len = sizeof(ct);
    CU_ASSERT_EQUAL(pqc_encapsulate(PQC_KYBER512, pk, params.public_key_size, ct, &ct_len, ss, sizeof(ss), NULL, 0), PQC_SUCCESS);
    CU_ASSERT_EQUAL(pqc_decapsulate(PQC_KYBER512, sk, params.secret_key_size, ct, ct_len, ss2, sizeof(ss2)), PQC_SUCCESS);
    CU_ASSERT_TRUE(memcmp(ss, ss2, 32) == 0);
}

static void test_pqc_error_handling(void) {
    pqc_params_t params;
    CU_ASSERT_NOT_EQUAL(pqc_get_params((pqc_algorithm_t)999, &params), PQC_SUCCESS);
    CU_ASSERT_EQUAL(pqc_keygen(PQC_KYBER512, NULL, 100, sk, 100, NULL, 0), PQC_ERROR_INVALID_PARAM);
}

static void test_pqc_error_strings(void) {
    const char *s = pqc_result_to_string(PQC_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(s);
    CU_ASSERT_STRING_EQUAL(s, "Success");
}

int init_pqc_tests(void) {
    CU_pSuite suite = CU_add_suite("PqCSuite", NULL, NULL);
    if (!suite) return CU_get_error();
    CU_ADD_TEST(suite, test_pqc_kyber768_params);
    CU_ADD_TEST(suite, test_pqc_kyber_roundtrip);
    CU_ADD_TEST(suite, test_pqc_error_handling);
    CU_ADD_TEST(suite, test_pqc_error_strings);
    return CUE_SUCCESS;
}
