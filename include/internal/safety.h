/*
 * Safety macros and overflow-safe arithmetic
 *
 * V002: NULL pointer validation
 * V001: Integer overflow protection
 */

#ifndef NACL_SAFETY_H
#define NACL_SAFETY_H

#include <stddef.h>
#include <stdint.h>

/* V002: NULL pointer validation */
#define NACL_CHECK_PTR(p)                                                                          \
    do {                                                                                           \
        if ((p) == NULL)                                                                           \
            return -1;                                                                             \
    } while (0)

/* V001: Safe unsigned addition with overflow check */
static inline int nacl_add_u64_overflow(uint64_t a, uint64_t b, uint64_t *result) {
    if (a > UINT64_MAX - b)
        return -1;
    *result = a + b;
    return 0;
}

static inline int nacl_add_size_overflow(size_t a, size_t b, size_t *result) {
    if (a > SIZE_MAX - b)
        return -1;
    *result = a + b;
    return 0;
}

#endif /* NACL_SAFETY_H */
