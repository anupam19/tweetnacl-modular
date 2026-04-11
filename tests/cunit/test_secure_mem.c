/*
 * Secure Memory Tests (CUnit)
 * Migrated from tests/test_secure_mem.c
 * Tests for CERT C compliant memory operations
 * 
 * Author: Anupam Datta <adbd04@gmail.com>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../../includes/secure_mem.h"
#include "../../includes/secure_utils.h"

static int secure_mem_setup(void) { return 0; }
static int secure_mem_teardown(void) { return 0; }

/* Test: secure_memset basic functionality */
static void test_secure_memset_basic(void) {
    unsigned char buffer[32];
    memset(buffer, 0x55, sizeof(buffer));
    
    secure_memset(buffer, 0x00, sizeof(buffer));
    
    for (size_t i = 0; i < sizeof(buffer); i++) {
        CU_ASSERT(0x00 == buffer[i]);
    }
}

/* Test: secure_zero clears sensitive data */
static void test_secure_zero(void) {
    unsigned char sensitive[64];
    memset(sensitive, 0xAA, sizeof(sensitive));
    
    secure_zero(sensitive, sizeof(sensitive));
    
    for (size_t i = 0; i < sizeof(sensitive); i++) {
        CU_ASSERT(0x00 == sensitive[i]);
    }
}

/* Test: safe_memcpy bounds checking */
static void test_safe_memcpy_bounds(void) {
    unsigned char dest[32];
    unsigned char src[64];
    memset(src, 0x42, sizeof(src));
    
    /* Should succeed - within bounds */
    CU_ASSERT(0 == safe_memcpy(dest, sizeof(dest), src, 16));
    
    /* Should fail - exceeds bounds */
    CU_ASSERT(-1 == safe_memcpy(dest, sizeof(dest), src, 64));
}

/* Test: safe_memcpy NULL pointer handling */
static void test_safe_memcpy_null(void) {
    unsigned char buffer[32];
    
    CU_ASSERT(-1 == safe_memcpy(NULL, sizeof(buffer), buffer, 16));
    CU_ASSERT(-1 == safe_memcpy(buffer, sizeof(buffer), NULL, 16));
}

/* Test: secure_memcmp constant-time comparison */
static void test_secure_memcmp(void) {
    unsigned char a[32], b[32], c[32];
    memset(a, 0x41, sizeof(a));
    memset(b, 0x41, sizeof(b));
    memset(c, 0x42, sizeof(c));
    
    /* Equal buffers should return 0 */
    CU_ASSERT(0 == secure_memcmp(a, b, sizeof(a)));
    
    /* Different buffers should return non-zero */
    CU_ASSERT(-1 == secure_memcmp(a, c, sizeof(a)));
}

/* Test: safe_strcpy null termination */
static void test_safe_strcpy_termination(void) {
    char dest[16];
    const char* src = "Hello";
    
    CU_ASSERT(0 == safe_strcpy(dest, sizeof(dest), src));
    CU_ASSERT_STRING_EQUAL("Hello", dest);
}

/* Test: safe_strcpy truncation */
static void test_safe_strcpy_truncation(void) {
    char dest[8];
    const char* src = "Hello World";
    
    /* Should fail due to truncation but still null-terminate */
    int result = safe_strcpy(dest, sizeof(dest), src);
    CU_ASSERT(-1 == result);
    CU_ASSERT('\0' == dest[sizeof(dest) - 1]);
}

/* Test: safe_add_size overflow detection */
static void test_safe_add_overflow(void) {
    size_t result;
    
    /* Should succeed */
    CU_ASSERT(0 == safe_add_size(100, 200, &result));
    CU_ASSERT(300 == result);
    
    /* Should fail - overflow */
    CU_ASSERT(-1 == safe_add_size(SIZE_MAX, 1, &result));
}

/* Test: safe_mul_size overflow detection */
static void test_safe_mul_overflow(void) {
    size_t result;
    
    /* Should succeed */
    CU_ASSERT(0 == safe_mul_size(100, 200, &result));
    CU_ASSERT(20000 == result);
    
    /* Should fail - overflow */
    CU_ASSERT(-1 == safe_mul_size(SIZE_MAX, 2, &result));
}

/* Test: validate_index bounds */
static void test_validate_index(void) {
    /* Valid index */
    CU_ASSERT(0 == validate_index(5, 10));
    
    /* Invalid index - at boundary */
    CU_ASSERT(-1 == validate_index(10, 10));
    
    /* Invalid index - beyond boundary */
    CU_ASSERT(-1 == validate_index(15, 10));
}

/* Test: constant_time_select */
static void test_constant_time_select(void) {
    uint8_t result;
    
    result = constant_time_select(1, 0xAA, 0xBB);
    CU_ASSERT(0xAA == result);
    
    result = constant_time_select(0, 0xAA, 0xBB);
    CU_ASSERT(0xBB == result);
}

/* Test: secure_memory_lock/unlock */
static void test_memory_lock(void) {
    unsigned char buffer[4096];
    
    /* Try to lock memory (may fail on some systems without privileges) */
    int lock_result = secure_memory_lock(buffer, sizeof(buffer));
    
    /* Unlock regardless of lock success */
    int unlock_result = secure_memory_unlock(buffer, sizeof(buffer));
    
    /* If lock succeeded, unlock should succeed */
    if (lock_result == 0) {
        CU_ASSERT(0 == unlock_result);
    }
}

/* Test: safe_range_check validation */
static void test_safe_range_check(void) {
    /* Valid range */
    CU_ASSERT(0 == safe_range_check(5, 0, 10));
    
    /* Below range */
    CU_ASSERT(-1 == safe_range_check(-1, 0, 10));
    
    /* Above range */
    CU_ASSERT(-1 == safe_range_check(11, 0, 10));
}

CU_ErrorCode create_secure_mem_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("SecureMemSuite", secure_mem_setup, secure_mem_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    CU_add_test(suite, "test_secure_memset_basic", test_secure_memset_basic);
    CU_add_test(suite, "test_secure_zero", test_secure_zero);
    CU_add_test(suite, "test_safe_memcpy_bounds", test_safe_memcpy_bounds);
    CU_add_test(suite, "test_safe_memcpy_null", test_safe_memcpy_null);
    CU_add_test(suite, "test_secure_memcmp", test_secure_memcmp);
    CU_add_test(suite, "test_safe_strcpy_termination", test_safe_strcpy_termination);
    CU_add_test(suite, "test_safe_strcpy_truncation", test_safe_strcpy_truncation);
    CU_add_test(suite, "test_safe_add_overflow", test_safe_add_overflow);
    CU_add_test(suite, "test_safe_mul_overflow", test_safe_mul_overflow);
    CU_add_test(suite, "test_validate_index", test_validate_index);
    CU_add_test(suite, "test_constant_time_select", test_constant_time_select);
    CU_add_test(suite, "test_memory_lock", test_memory_lock);
    CU_add_test(suite, "test_safe_range_check", test_safe_range_check);
    
    return CUE_SUCCESS;
}
