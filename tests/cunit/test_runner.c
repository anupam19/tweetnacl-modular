/*
 * TweetNaCl-Modular CUnit Test Runner
 * Main entry point for CUnit test framework
 * 
 * Author: Anupam Datta <adbd04@gmail.com>
 */

#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include <stdio.h>
#include <stdlib.h>

/* Test suite declarations */
CU_ErrorCode create_core_suite(void);
CU_ErrorCode create_box_suite(void);
CU_ErrorCode create_sign_suite(void);
CU_ErrorCode create_secretbox_suite(void);
CU_ErrorCode create_pqc_suite(void);
CU_ErrorCode create_secure_mem_suite(void);
CU_ErrorCode create_arch_suite(void);
CU_ErrorCode create_constant_time_suite(void);

int main(int argc, char **argv) {
    CU_ErrorCode result;
    
    /* Initialize CUnit registry */
    if (CUE_SUCCESS != CU_initialize_registry()) {
        fprintf(stderr, "Failed to initialize CUnit registry\n");
        return CU_get_error();
    }
    
    /* Create all test suites */
    result = create_core_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    result = create_box_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    result = create_sign_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    result = create_secretbox_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    result = create_pqc_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    result = create_secure_mem_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    result = create_arch_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    result = create_constant_time_suite();
    if (result != CUE_SUCCESS) goto cleanup;
    
    /* Run tests */
    printf("========================================\n");
    printf("TweetNaCl-Modular CUnit Test Suite\n");
    printf("========================================\n\n");
    
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    int failures = CU_get_number_of_failures();
    int tests_run = CU_get_number_of_tests_run();
    
    printf("\n========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Total tests run: %d\n", tests_run);
    printf("Failures: %d\n", failures);
    printf("========================================\n");
    
cleanup:
    CU_cleanup_registry();
    return (failures > 0 || result != CUE_SUCCESS) ? 1 : 0;
}
