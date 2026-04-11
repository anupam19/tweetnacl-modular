/*
 * Constant-Time Tests (CUnit)
 * Statistical tests for timing attack resistance
 * 
 * Author: Anupam Datta <adbd04@gmail.com>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "../../includes/secure_mem.h"
#include "../../includes/tweetnacl.h"

static int ct_setup(void) { return 0; }
static int ct_teardown(void) { return 0; }

/* Helper: Get current time in nanoseconds */
static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/* Test: secure_memcmp timing consistency */
static void test_secure_memcmp_timing(void) {
    unsigned char a[64], b[64], c[64];
    uint64_t times[10];
    uint64_t min_time, max_time;
    int i;
    
    memset(a, 0x41, sizeof(a));
    memset(b, 0x41, sizeof(b));  /* Equal to a */
    memset(c, 0x42, sizeof(c));  /* Different from a */
    
    /* Time multiple comparisons with equal buffers */
    for (i = 0; i < 5; i++) {
        uint64_t start = get_time_ns();
        volatile int result = secure_memcmp(a, b, sizeof(a));
        (void)result;
        times[i] = get_time_ns() - start;
    }
    
    /* Time multiple comparisons with different buffers */
    for (i = 5; i < 10; i++) {
        uint64_t start = get_time_ns();
        volatile int result = secure_memcmp(a, c, sizeof(a));
        (void)result;
        times[i] = get_time_ns() - start;
    }
    
    /* Find min and max times */
    min_time = times[0];
    max_time = times[0];
    for (i = 1; i < 10; i++) {
        if (times[i] < min_time) min_time = times[i];
        if (times[i] > max_time) max_time = times[i];
    }
    
    /* Timing variance should be within reasonable bounds */
    /* Note: This is a statistical test and may have false positives */
    CU_ASSERT(max_time < min_time * 10);  /* Within 10x variance */
}

/* Test: crypto_verify_32 timing consistency */
static void test_crypto_verify_timing(void) {
    uint8_t a[32], b[32], c[32];
    uint64_t times[10];
    uint64_t min_time, max_time;
    int i;
    
    memset(a, 0x41, sizeof(a));
    memset(b, 0x41, sizeof(b));
    memset(c, 0x42, sizeof(c));
    
    /* Time equal comparisons */
    for (i = 0; i < 5; i++) {
        uint64_t start = get_time_ns();
        volatile int result = crypto_verify_32(a, b);
        (void)result;
        times[i] = get_time_ns() - start;
    }
    
    /* Time different comparisons */
    for (i = 5; i < 10; i++) {
        uint64_t start = get_time_ns();
        volatile int result = crypto_verify_32(a, c);
        (void)result;
        times[i] = get_time_ns() - start;
    }
    
    min_time = times[0];
    max_time = times[0];
    for (i = 1; i < 10; i++) {
        if (times[i] < min_time) min_time = times[i];
        if (times[i] > max_time) max_time = times[i];
    }
    
    CU_ASSERT(max_time < min_time * 10);
}

/* Test: secure_zero does not optimize away */
static void test_secure_zero_effective(void) {
    volatile unsigned char buffer[32];
    int i;
    
    /* Fill with pattern */
    for (i = 0; i < 32; i++) {
        buffer[i] = 0xAA;
    }
    
    /* Zero it */
    secure_zero((void*)buffer, 32);
    
    /* Check it's actually zeroed (volatile prevents optimization) */
    for (i = 0; i < 32; i++) {
        CU_ASSERT(0x00 == buffer[i]);
    }
}

CU_ErrorCode create_constant_time_suite(void) {
    CU_pSuite suite;
    
    suite = CU_create_suite("ConstantTimeSuite", ct_setup, ct_teardown);
    if (NULL == suite) {
        return CU_get_error();
    }
    
    CU_add_test(suite, "test_secure_memcmp_timing", test_secure_memcmp_timing);
    CU_add_test(suite, "test_crypto_verify_timing", test_crypto_verify_timing);
    CU_add_test(suite, "test_secure_zero_effective", test_secure_zero_effective);
    
    return CUE_SUCCESS;
}
