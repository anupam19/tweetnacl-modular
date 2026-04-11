/*
 * Random Bytes Generation Header
 * Provides cryptographically secure random number generation
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 */

#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generate cryptographically secure random bytes
 * @param buf Output buffer
 * @param len Number of bytes to generate
 */
void randombytes(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* RANDOMBYTES_H */
