/**
 * CUnit Test Runner for TweetNaCl-Modular
 * 
 * Main entry point for running all CUnit test suites.
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>

/* Forward declarations of suite registration functions */
extern void register_core_suite(CU_pRegistry registry);
extern void register_box_suite(CU_pRegistry registry);
extern void register_sign_suite(CU_pRegistry registry);
extern void register_secretbox_suite(CU_pRegistry registry);
extern void register_secure_mem_suite(CU_pRegistry registry);

int main(int argc, char *argv[]) {
    CU_ErrorCode result;
    
    /* Initialize the CUnit test registry */
    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf(stderr, "Failed to initialize CUnit registry\\n");
        return EXIT_FAILURE;
    }
    
    /* Register all test suites */
    register_core_suite(CU_get_registry());
    register_box_suite(CU_get_registry());
    register_sign_suite(CU_get_registry());
    register_secretbox_suite(CU_get_registry());
    register_secure_mem_suite(CU_get_registry());
    
    /* Run all tests with basic mode (minimal output) */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    
    printf("========================================\\n");
    printf("TweetNaCl-Modular CUnit Tests\\n");
    printf("========================================\\n\\n");
    
    result = CU_basic_run_tests();
    
    printf("\\n========================================\\n");
    printf("Test Run Complete\\n");
    printf("========================================\\n");
    
    /* Cleanup and exit */
    CU_cleanup_registry();
    
    return (result == CUE_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
