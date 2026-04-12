/*
 * TweetNaCl-Modular CUnit Test Runner
 * Main entry point for CUnit automated test suite
 */

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

/* Test suite initialization functions */
extern int init_core_tests(void);
extern int init_box_tests(void);
extern int init_sign_tests(void);
extern int init_secretbox_tests(void);
extern int init_pqc_tests(void);
extern int init_secure_mem_tests(void);
extern int init_arch_tests(void);
extern int init_constant_time_tests(void);

int main(void) {
    CU_ErrorCode err;

    /* Initialize CUnit registry */
    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf(stderr, "CUnit initialization failed\n");
        return 1;
    }

    /* Register all test suites */
    if ((err = init_core_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "Core tests init failed: %d\n", err);
        return 1;
    }
    if ((err = init_box_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "Box tests init failed: %d\n", err);
        return 1;
    }
    if ((err = init_sign_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "Sign tests init failed: %d\n", err);
        return 1;
    }
    if ((err = init_secretbox_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "SecretBox tests init failed: %d\n", err);
        return 1;
    }
    if ((err = init_pqc_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "PQC tests init failed: %d\n", err);
        return 1;
    }
    if ((err = init_secure_mem_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "Secure mem tests init failed: %d\n", err);
        return 1;
    }
    if ((err = init_arch_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "Arch tests init failed: %d\n", err);
        return 1;
    }
    if ((err = init_constant_time_tests()) != CUE_SUCCESS) {
        fprintf(stderr, "Constant time tests init failed: %d\n", err);
        return 1;
    }

    /* Run tests in Basic mode (output to stdout) */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    int failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    return (failures == 0) ? 0 : 1;
}
