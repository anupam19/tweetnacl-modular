/*
 * CUnit Tests: Secure Memory Operations
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "../includes/secure_mem.h"
#include "../includes/secure_utils.h"

static void test_secure_zero(void) {
    unsigned char buf[64];
    memset(buf, 0xAA, sizeof(buf));
    secure_zero(buf, sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++) CU_ASSERT(buf[i] == 0);
}

static void test_secure_memcmp_equal(void) {
    uint8_t a[32], b[32];
    memset(a, 0x42, 32); memcpy(b, a, 32);
    CU_ASSERT_EQUAL(secure_memcmp(a, b, 32), 0);
}

static void test_secure_memcmp_not_equal(void) {
    uint8_t a[32], b[32];
    memset(a, 0x42, 32); memset(b, 0x42, 32); b[16] ^= 0x01;
    CU_ASSERT_NOT_EQUAL(secure_memcmp(a, b, 32), 0);
}

static void test_safe_memcpy_bounds(void) {
    uint8_t dst[32], src[64];
    memset(src, 0x42, sizeof(src));
    CU_ASSERT_EQUAL(safe_memcpy(dst, sizeof(dst), src, 16), 0);
    CU_ASSERT_NOT_EQUAL(safe_memcpy(dst, sizeof(dst), src, 64), 0);
}

static void test_safe_memcpy_null(void) {
    uint8_t buf[32];
    CU_ASSERT_NOT_EQUAL(safe_memcpy(NULL, sizeof(buf), buf, 16), 0);
    CU_ASSERT_NOT_EQUAL(safe_memcpy(buf, sizeof(buf), NULL, 16), 0);
}

static void test_safe_add_overflow(void) {
    size_t r;
    CU_ASSERT_EQUAL(safe_add_size(100, 200, &r), 0);
    CU_ASSERT_EQUAL(r, 300);
    CU_ASSERT_NOT_EQUAL(safe_add_size(SIZE_MAX, 1, &r), 0);
}

static void test_constant_time_select(void) {
    CU_ASSERT_EQUAL(constant_time_select(1, 0xAA, 0xBB), 0xAA);
    CU_ASSERT_EQUAL(constant_time_select(0, 0xAA, 0xBB), 0xBB);
}

int init_secure_mem_tests(void) {
    CU_pSuite suite = CU_add_suite("SecureMemSuite", NULL, NULL);
    if (!suite) return CU_get_error();
    CU_ADD_TEST(suite, test_secure_zero);
    CU_ADD_TEST(suite, test_secure_memcmp_equal);
    CU_ADD_TEST(suite, test_secure_memcmp_not_equal);
    CU_ADD_TEST(suite, test_safe_memcpy_bounds);
    CU_ADD_TEST(suite, test_safe_memcpy_null);
    CU_ADD_TEST(suite, test_safe_add_overflow);
    CU_ADD_TEST(suite, test_constant_time_select);
    return CUE_SUCCESS;
}
