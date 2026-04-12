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

/**
 * Check if hardware DRNG (Digital Random Number Generator) is available
 * @return 1 if hardware RNG is available, 0 otherwise (software fallback)
 */
int randombytes_drng_available(void);

/**
 * Get the name of the current random bytes implementation
 * @return String identifying the RNG source (e.g., "RDRAND", "RDSEED",
 *         "ARM_RNDR", "BCryptGenRandom", "/dev/urandom")
 */
const char *randombytes_implementation_name(void);

#ifdef __cplusplus
}
#endif

#endif /* RANDOMBYTES_H */
