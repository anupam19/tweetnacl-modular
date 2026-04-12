/*
 * CUnit Tests: Architecture Detection
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../arch/arch_config.h"

static void test_arch_name_defined(void) {
    CU_ASSERT_PTR_NOT_NULL(ARCH_NAME);
    CU_ASSERT_STRING_NOT_EQUAL(ARCH_NAME, "");
}

static void test_endianness_detected(void) {
    /* Should have exactly one endianness defined */
#ifdef ARCH_LITTLE_ENDIAN
    CU_ASSERT_TRUE(1);
#elif defined(ARCH_BIG_ENDIAN)
    CU_ASSERT_TRUE(1);
#else
    CU_FAIL("No endianness detected");
#endif
}

static void test_uint64_available(void) {
    CU_ASSERT_TRUE(HAVE_NATIVE_UINT64 == 1);
}

int init_arch_tests(void) {
    CU_pSuite suite = CU_add_suite("ArchSuite", NULL, NULL);
    if (!suite) return CU_get_error();
    CU_ADD_TEST(suite, test_arch_name_defined);
    CU_ADD_TEST(suite, test_endianness_detected);
    CU_ADD_TEST(suite, test_uint64_available);
    return CUE_SUCCESS;
}
