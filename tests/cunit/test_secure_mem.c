/**
 * CUnit Tests for Secure Memory Functions
 * 
 * Tests for secure_zero, secure_memset, secure_memcmp.
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "../../includes/secure_mem.h"

void register_secure_mem_suite(CU_pRegistry registry) {
    CU_pSuite suite = NULL;
    
    /* Create the SecureMemSuite */
    suite = CU_add_suite(registry, "SecureMemSuite", NULL, NULL);
    if (suite == NULL) {
        CU_cleanup_registry();
        return;
    }
    
    /* Add tests to SecureMemSuite */
    CU_add_test(suite, "test_secure_zero_basic", NULL);
    CU_add_test(suite, "test_secure_memset_basic", NULL);
    CU_add_test(suite, "test_secure_memcmp_equal", NULL);
    CU_add_test(suite, "test_secure_memcmp_different", NULL);
}
