/*
 * Secure Memory Operations Implementation
 * Provides hardened memset, memcpy, and other memory operations
 * 
 * CERT C Compliance:
 * - MEM35-C: Allocate sufficient memory for an object
 * - ARR30-C: Do not form or use out-of-bounds pointers
 * - SEC30-C: Ensure that pointer data is not used after being freed
 */

#include "secure_mem.h"
#include <string.h>
#include <errno.h>

/* Compiler barrier to prevent optimization removal */
#if defined(__GNUC__) || defined(__clang__)
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#elif defined(_MSC_VER)
#define COMPILER_BARRIER() _ReadWriteBarrier()
#else
#define COMPILER_BARRIER() ((void)0)
#endif

/**
 * Secure memset - cannot be optimized away by compiler
 * Uses volatile access and compiler barriers
 */
void secure_memset(volatile void* dest, int value, size_t count) {
    if (dest == NULL || count == 0) {
        return;
    }
    
    volatile unsigned char* p = (volatile unsigned char*)dest;
    while (count--) {
        *p++ = (unsigned char)value;
    }
    
    /* Prevent compiler from optimizing away the memset */
    COMPILER_BARRIER();
}

/**
 * Secure zero - specialized wrapper for clearing sensitive data
 */
void secure_zero(volatile void* dest, size_t count) {
    secure_memset(dest, 0, count);
}

/**
 * Safe memcpy with bounds checking
 */
int safe_memcpy(void* dest, size_t dest_size, const void* src, size_t count) {
    if (dest == NULL || src == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    if (count > dest_size) {
        errno = ERANGE;
        return -1;
    }
    
    memcpy(dest, src, count);
    return 0;
}

/**
 * Safe memmove with bounds checking (handles overlapping regions)
 */
int safe_memmove(void* dest, size_t dest_size, const void* src, size_t count) {
    if (dest == NULL || src == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    if (count > dest_size) {
        errno = ERANGE;
        return -1;
    }
    
    memmove(dest, src, count);
    return 0;
}

/**
 * Constant-time memory comparison
 * Prevents timing attacks by always comparing all bytes
 */
int secure_memcmp(const void* x, const void* y, size_t n) {
    const volatile unsigned char* vx = (const volatile unsigned char*)x;
    const volatile unsigned char* vy = (const volatile unsigned char*)y;
    unsigned char result = 0;
    size_t i;
    
    if (x == NULL || y == NULL) {
        return -1;
    }
    
    for (i = 0; i < n; i++) {
        result |= vx[i] ^ vy[i];
    }
    
    return (result != 0) ? -1 : 0;
}
