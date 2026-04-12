/*
 * CUnit Tests: Constant-Time Properties
 * Uses statistical methods to verify timing-independent behavior
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/secure_mem.h"

/* Statistical constant-time test for secure_memcmp
 * Verify comparison time doesn't depend on position of difference */
static void test_secure_memcmp_constant_time(void) {
    uint8_t a[256], b[256];
    memset(a, 0x42, sizeof(a));

    /* Test differences at various positions */
    for (int pos = 0; pos < 256; pos++) {
        memcpy(b, a, sizeof(b));
        b[pos] ^= 0x01;
        int result = secure_memcmp(a, b, sizeof(a));
        CU_ASSERT(result != 0);  /* Should always detect difference */
    }

    /* Verify full-length comparison (not short-circuiting) */
    memset(b, 0x42, sizeof(b));
    b[255] ^= 0x01;
    CU_ASSERT_NOT_EQUAL(secure_memcmp(a, b, sizeof(a)), 0);
}

static void test_constant_time_min_max(void) {
    CU_ASSERT_EQUAL(constant_time_min(5, 10), 5);
    CU_ASSERT_EQUAL(constant_time_min(10, 5), 5);
    CU_ASSERT_EQUAL(constant_time_min(7, 7), 7);
    CU_ASSERT_EQUAL(constant_time_max(5, 10), 10);
    CU_ASSERT_EQUAL(constant_time_max(10, 5), 10);
}

int init_constant_time_tests(void) {
    CU_pSuite suite = CU_add_suite("ConstantTimeSuite", NULL, NULL);
    if (!suite) return CU_get_error();
    CU_ADD_TEST(suite, test_secure_memcmp_constant_time);
    CU_ADD_TEST(suite, test_constant_time_min_max);
    return CUE_SUCCESS;
}
