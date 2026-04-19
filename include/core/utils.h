/*
 * Secure Coding Utilities Header
 * Provides CERT C compliant utility functions
 *
 * CERT C Rules Applied:
 * - INT30-C: Ensure that unsigned integer operations do not wrap
 * - INT32-C: Ensure that operations on signed integers do not result in overflow
 * - INT35-C: Use correct expressions for object sizes in pointer comparisons
 */

#ifndef SECURE_UTILS_H
#define SECURE_UTILS_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Safe addition with overflow checking
 * @param a First operand
 * @param b Second operand
 * @param result Output (if successful)
 * @return 0 on success, -1 on overflow
 */
int safe_add_int(int a, int b, int *result);

/**
 * Safe subtraction with underflow checking
 * @param a First operand
 * @param b Second operand
 * @param result Output (if successful)
 * @return 0 on success, -1 on underflow
 */
int safe_sub_int(int a, int b, int *result);

/**
 * Safe multiplication with overflow checking
 * @param a First operand
 * @param b Second operand
 * @param result Output (if successful)
 * @return 0 on success, -1 on overflow
 */
int safe_mul_int(int a, int b, int *result);

/**
 * Safe size_t addition
 * @param a First operand
 * @param b Second operand
 * @param result Output (if successful)
 * @return 0 on success, -1 on overflow
 */
int safe_add_size(size_t a, size_t b, size_t *result);

/**
 * Constant-time conditional select
 * Prevents timing attacks by avoiding branches
 * @param condition Condition value (0 or non-zero)
 * @param if_true Value to return if condition is true
 * @param if_false Value to return if condition is false
 * @return Selected value
 */
uint8_t constant_time_select(uint8_t condition, uint8_t if_true, uint8_t if_false);

/**
 * Constant-time minimum
 * @param a First value
 * @param b Second value
 * @return Minimum of a and b
 */
size_t constant_time_min(size_t a, size_t b);

/**
 * Constant-time maximum
 * @param a First value
 * @param b Second value
 * @return Maximum of a and b
 */
size_t constant_time_max(size_t a, size_t b);

/**
 * Validate that a value is within a safe range
 * @param value Value to check
 * @param min_val Minimum allowed value
 * @param max_val Maximum allowed value
 * @return 0 if in range, -1 if out of range
 */
int safe_range_check(int value, int min_val, int max_val);

/**
 * Safe size_t multiplication with overflow checking
 * @param a First operand
 * @param b Second operand
 * @param result Output (if successful)
 * @return 0 on success, -1 on overflow
 */
int safe_mul_size(size_t a, size_t b, size_t *result);

/**
 * Validate array index bounds
 * @param index Index to validate
 * @param array_size Size of the array
 * @return 0 if valid, -1 if out of bounds
 */
int validate_index(size_t index, size_t array_size);

/**
 * @brief Ensure a pointer is non-null; return -1 if null.
 * @param ptr Pointer to validate
 *
 * Provides consistent NULL checking across the codebase.
 * Usage: NACL_ENSURE(pointer);
 */
#define NACL_ENSURE(ptr) do { if ((ptr) == NULL) return -1; } while(0)

#ifdef __cplusplus
}
#endif

#endif /* SECURE_UTILS_H */
