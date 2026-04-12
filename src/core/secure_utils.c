/*
 * Secure Coding Utilities Implementation
 * Provides CERT C compliant utility functions
 *
 * CERT C Rules Applied:
 * - INT30-C: Ensure that unsigned integer operations do not wrap
 * - INT32-C: Ensure that operations on signed integers do not result in overflow
 * - INT35-C: Use correct expressions for object sizes in pointer comparisons
 */

#include "api/secure_utils.h"
#include <errno.h>
#include <sys/types.h>

int safe_add_int(int a, int b, int* result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Check for overflow before it happens */
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) {
        errno = ERANGE;
        return -1;
    }
    
    *result = a + b;
    return 0;
}

int safe_sub_int(int a, int b, int* result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Check for underflow before it happens */
    if ((b > 0 && a < INT_MIN + b) || (b < 0 && a > INT_MAX + b)) {
        errno = ERANGE;
        return -1;
    }
    
    *result = a - b;
    return 0;
}

int safe_mul_int(int a, int b, int* result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Check for multiplication overflow */
    if (a > 0) {
        if (b > 0) {
            if (a > INT_MAX / b) {
                errno = ERANGE;
                return -1;
            }
        } else {
            if (b < INT_MIN / a) {
                errno = ERANGE;
                return -1;
            }
        }
    } else {
        if (b > 0) {
            if (a < INT_MIN / b) {
                errno = ERANGE;
                return -1;
            }
        } else {
            if (a != 0 && b < INT_MAX / a) {
                errno = ERANGE;
                return -1;
            }
        }
    }
    
    *result = a * b;
    return 0;
}

int safe_add_size(size_t a, size_t b, size_t* result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Check for overflow */
    if (a > SIZE_MAX - b) {
        errno = ERANGE;
        return -1;
    }
    
    *result = a + b;
    return 0;
}

uint8_t constant_time_select(uint8_t condition, uint8_t if_true, uint8_t if_false) {
    /* Convert condition to all-ones (0xFF) or all-zeros (0x00) */
    uint8_t mask = (uint8_t)(-(condition != 0));
    return (if_true & mask) | (if_false & ~mask);
}

size_t constant_time_min(size_t a, size_t b) {
    /* Branchless minimum using bitwise operations */
    size_t diff = a - b;
    size_t mask = (size_t)((ssize_t)diff >> (sizeof(size_t) * 8 - 1));
    return b ^ (mask & (a ^ b));
}

size_t constant_time_max(size_t a, size_t b) {
    /* Branchless maximum using bitwise operations */
    size_t diff = a - b;
    size_t mask = (size_t)((ssize_t)diff >> (sizeof(size_t) * 8 - 1));
    return a ^ (mask & (a ^ b));
}

int safe_range_check(int value, int min_val, int max_val) {
    if (min_val > max_val) {
        return -1;
    }
    
    /* Constant-time range check */
    int below_min = (value < min_val);
    int above_max = (value > max_val);
    
    return (below_min | above_max) ? -1 : 0;
}
