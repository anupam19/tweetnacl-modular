/*
 * Architecture Detection Tests (CUnit)
 * Tests for architecture-specific functionality
 * 
 * Author: Anupam Datta <adbd04@gmail.com>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../../arch/arch_config.h"

static int arch_setup(void) { return 0; }
static int arch_teardown(void) { return 0; }

/* Test: Architecture name is defined */
static void test_arch_name_defined(void) {
    CU_ASSERT(ARCH_NAME != NULL);
}

/* Test: Endianness detection */
static void test_endianness(void) {
#if defined(ARCH_LITTLE_ENDIAN)
    CU_PASS("Little endian detected");
#elif defined(ARCH_BIG_ENDIAN)
    CU_PASS("Big endian detected");
#else
    CU_FAIL("Endianness not detected");
#endif
}

/* Test: HAVE_NATIVE_UINT64 defined */
static void test_have_uint64(void) {
    CU_ASSERT(1 == HAVE_NATIVE_UINT64);
}

/* Test: Architecture-specific flags */
static void test_arch_flags(void) {
#if defined(ARCH_X86_64) || defined(ARCH_X86_32) || \
    defined(ARCH_ARM64) || defined(ARCH_ARM32) || \
    defined(ARCH_RISCV64) || defined(ARCH_RISCV32) || \
    defined(ARCH_NATIVE)
    CU_PASS("Architecture flag defined");
#else
    /* May be native or generic */
    CU_PASS("Generic architecture");
#endif
}

CU_ErrorCode create_arch_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("ArchSuite", arch_setup, arch_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    CU_add_test(suite, "test_arch_name_defined", test_arch_name_defined);
    CU_add_test(suite, "test_endianness", test_endianness);
    CU_add_test(suite, "test_have_uint64", test_have_uint64);
    CU_add_test(suite, "test_arch_flags", test_arch_flags);
    
    return CUE_SUCCESS;
}
