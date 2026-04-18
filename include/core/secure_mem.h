/*
 * Secure Memory Operations Header
 * Provides hardened memset, memcpy, and other memory operations
 *
 * CERT C Compliance:
 * - MEM35-C: Allocate sufficient memory for an object
 * - ARR30-C: Do not form or use out-of-bounds pointers
 * - SEC30-C: Ensure that pointer data is not used after being freed
 */

#ifndef SECURE_MEM_H
#define SECURE_MEM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Secure memset - cannot be optimized away by compiler
 * @param dest Destination buffer
 * @param value Value to set
 * @param count Number of bytes
 */
void secure_memset(volatile void *dest, int value, size_t count);

/**
 * Secure zero - specialized wrapper for clearing sensitive data
 * @param dest Destination buffer
 * @param count Number of bytes
 */
void secure_zero(volatile void *dest, size_t count);

/**
 * Safe memcpy with bounds checking
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source buffer
 * @param count Number of bytes to copy
 * @return 0 on success, -1 on error
 */
int safe_memcpy(void *dest, size_t dest_size, const void *src, size_t count);

/**
 * Safe memmove with bounds checking (handles overlapping regions)
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source buffer
 * @param count Number of bytes to move
 * @return 0 on success, -1 on error
 */
int safe_memmove(void *dest, size_t dest_size, const void *src, size_t count);

/**
 * Constant-time memory comparison
 * @param x First buffer
 * @param y Second buffer
 * @param n Number of bytes to compare
 * @return 0 if equal, -1 if different
 */
int secure_memcmp(const void *x, const void *y, size_t n);

/**
 * Safe string copy with null termination
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source string
 * @return 0 if successful, 1 if truncated, -1 on error
 */
int safe_strcpy(char *dest, size_t dest_size, const char *src);

/**
 * Lock memory pages to prevent swapping
 * @param addr Memory address
 * @param len Length in bytes
 * @return 0 on success, -1 on failure
 */
int secure_memory_lock(void *addr, size_t len);

/**
 * Unlock memory pages (allows swapping)
 * @param addr Memory address
 * @param len Length in bytes
 * @return 0 on success, -1 on failure
 */
int secure_memory_unlock(void *addr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SECURE_MEM_H */
