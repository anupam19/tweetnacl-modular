/*
 * DRNG (Digital Random Number Generator) Interface
 *
 * Inspired by Intel DRNG samples (Intel Corp, 2014, BSD-3)
 * Adapted for TweetNaCl-Modular architecture
 * 
 * FIPS 140-3 Compliance:
 *   - Section 9.3: Power-up self-tests
 *   - NIST 800-90B: Continuous health tests
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
 * @brief Check if hardware DRNG is available
 * @return 1 if hardware RNG is available, 0 otherwise
 */
int randombytes_drng_available(void);

/**
 * @brief Get DRNG feature flags
 * @return Bitmask of DRNG_HAS_RDRAND and/or DRNG_HAS_RDSEED
 */
int drng_get_drng_support(void);

/**
 * @brief Get the name of the current RNG implementation
 * @return String like "RDRAND", "RDSEED", "ARM_RNDR", "/dev/urandom"
 */
const char *randombytes_implementation_name(void);

/**
 * @brief Internal: Fill buffer with hardware RNG
 * @param[out] buf Buffer to fill with random data
 * @param[in] len Length of buffer
 * @return 0 on success, -1 on failure
 */
int _randombytes_drng_fill(uint8_t *buf, size_t len);

/**
 * @brief Get DRNG initialization status
 * @return 1 if initialized, 0 otherwise
 */
int drng_is_initialized_public(void);

/**
 * @brief Get power-up self-test status (FIPS 140-3)
 * @return 1 if passed, 0 if not passed or not tested
 */
int drng_get_power_up_selftest_status(void);

/**
 * @brief Get total bytes generated since initialization
 * @return Total byte count
 */
uint64_t drng_get_total_bytes_generated(void);

/**
 * @brief Reset DRNG state (for testing only)
 * @warning Should only be used in test environments
 */
void drng_reset_for_testing(void);

#ifdef __cplusplus
}
#endif

#endif /* DRNG_H */
