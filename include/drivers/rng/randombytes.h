/**
 * @file randombytes.h
 * @brief Random Bytes Generation Header
 * @details Provides cryptographically secure random number generation with
 *          proper error handling and multiple implementation backends.
 *
 * Priority order:
 *   1. Hardware DRNG (RDSEED/RDRAND/ARM RNDR) — if WITH_DRNG
 *   2. BCryptGenRandom (Windows CNG)
 *   3. /dev/urandom (POSIX fallback)
 *
 * NIST 800-90B: Periodic RNG health check every 1000 calls
 *
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 *
 * @copyright MIT License
 * @author Anupam Datta
 * @version 1.0.0
 * @date 2024
 *
 * @defgroup rng Random Number Generation
 * @brief Cryptographically secure random number generation
 * @{
 */

#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include "core/error.h"
#include <stddef.h>
#include <stdint.h>

/* Deprecation attribute */
#if defined(__clang__) || defined(__GNUC__)
#  define NACL_DEPRECATED __attribute__((deprecated("Use randombytes_safe() instead")))
#elif defined(_MSC_VER)
#  define NACL_DEPRECATED __declspec(deprecated("Use randombytes_safe() instead"))
#else
#  define NACL_DEPRECATED
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the random bytes generator
 * @return NACL_SUCCESS on success, error code on failure
 *
 * @note This function is optional - randombytes() will auto-initialize if needed
 * @note Thread-safe: can be called concurrently from multiple threads
 * @see randombytes_safe() for non-exiting variant
 */
int randombytes_init(void);

/**
 * @brief Generate cryptographically secure random bytes with error handling
 * @param[out] buf Output buffer
 * @param[in] len Number of bytes to generate
 * @return NACL_SUCCESS on success, error code on failure
 *
 * @note Uses hardware DRNG if available, falls back to OS RNG
 * @note Thread-safe: can be called concurrently from multiple threads
 * @warning Unlike legacy randombytes(), this function returns error codes
 *          instead of exiting on failure
 */
int randombytes_safe(uint8_t *buf, size_t len);

/**
 * @brief Generate cryptographically secure random bytes (legacy API)
 * @param[out] buf Output buffer
 * @param[in] len Number of bytes to generate
 *
 * @deprecated Use randombytes_safe() for proper error handling.
 *             This function no longer aborts; it returns on failure.
 *             It will be removed in a future major release.
 */
NACL_DEPRECATED void randombytes(uint8_t *buf, size_t len);

/**
 * @brief Check if hardware DRNG (Digital Random Number Generator) is available
 * @return 1 if hardware RNG is available, 0 otherwise (software fallback)
 */
int randombytes_drng_available(void);

/**
 * @brief Get the name of the current random bytes implementation
 * @return String identifying the RNG source (e.g., "RDRAND", "RDSEED",
 *         "ARM_RNDR", "BCryptGenRandom", "/dev/urandom")
 */
const char *randombytes_implementation_name(void);

#ifdef __cplusplus
}
#endif

#endif /* RANDOMBYTES_H */

/** @} */
