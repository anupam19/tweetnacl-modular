/*
 * DRNG (Digital Random Number Generator) Interface
 *
 * Inspired by Intel DRNG samples (Intel Corp, 2014, BSD-3)
 * Adapted for TweetNaCl-Modular architecture
 */

#ifndef DRNG_H
#define DRNG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DRNG support flags — OR'd together */
#define DRNG_NO_SUPPORT  0x0
#define DRNG_HAS_RDRAND  0x1
#define DRNG_HAS_RDSEED  0x2

/**
 * Check if hardware DRNG is available
 * @return 1 if hardware RNG is available, 0 otherwise
 */
int randombytes_drng_available(void);

/**
 * Get DRNG feature flags
 * @return Bitmask of DRNG_HAS_RDRAND and/or DRNG_HAS_RDSEED
 */
int drng_get_drng_support(void);

/**
 * Get the name of the current RNG implementation
 * @return String like "RDRAND", "RDSEED", "ARM_RNDR", "/dev/urandom"
 */
const char *randombytes_implementation_name(void);

/**
 * Internal: Fill buffer with hardware RNG
 * @return 0 on success, -1 on failure
 */
int _randombytes_drng_fill(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* DRNG_H */
