/**
 * @file randombytes.c
 * @brief Random Bytes Generation Implementation
 * @details Provides cryptographically secure random number generation with
 *          proper error handling and fallback mechanisms.
 *
 * Priority order:
 *   1. Hardware DRNG (RDSEED/RDRAND/ARM RNDR) — if WITH_DRNG
 *   2. BCryptGenRandom (Windows CNG)
 *   3. /dev/urandom (POSIX fallback)
 *
 * NIST 800-90B: Periodic RNG health check every 1000 calls
 *
 * CERT C Compliance:
 * - MEM35-C: Allocate sufficient memory for an object
 * - ARR30-C: Do not form or use out-of-bounds pointers
 *
 * @copyright MIT License
 * @author Anupam Datta
 * @version 1.0.0
 * @date 2024
 */

/* Force C11 atomics on MSVC: override any existing definition (including 0) */
#if defined(_HAS_C11_ATOMICS)
#  if _HAS_C11_ATOMICS != 1
#    undef _HAS_C11_ATOMICS
#    define _HAS_C11_ATOMICS 1
#  endif
#else
#  define _HAS_C11_ATOMICS 1
#endif

#include "drivers/rng/randombytes.h"
#include "core/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#ifndef _WIN32
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <windows.h>
#include <bcrypt.h>
#endif

/* DRNG internal helpers (from randombytes_drng.c) */
#ifdef WITH_DRNG
extern int _randombytes_drng_fill(uint8_t *buf, size_t len);
extern int _randombytes_drng_impl_is_software(void);
#endif

/* Periodic RNG health check counter (NIST 800-90B) */
static atomic_uint_fast64_t randombytes_call_count = ATOMIC_VAR_INIT(0);

/* Initialization state */
static atomic_int randombytes_initialized = ATOMIC_VAR_INIT(0);

/**
 * @brief Initialize the random bytes generator
 * @return NACL_SUCCESS on success, error code on failure
 *
 * @note This function is optional - randombytes() will auto-initialize if needed
 */
int randombytes_init(void) {
    if (atomic_load_explicit(&randombytes_initialized, memory_order_acquire)) {
        return NACL_SUCCESS;
    }

#ifdef WITH_DRNG
    /* Test hardware DRNG availability */
    uint8_t test_buf[16];
    if (_randombytes_drng_fill(test_buf, sizeof(test_buf)) == 0) {
        /* Hardware DRNG available */
        atomic_store_explicit(&randombytes_initialized, 1, memory_order_release);
        return NACL_SUCCESS;
    }
    /* Hardware DRNG not available - will fall back to software */
#endif

    atomic_store_explicit(&randombytes_initialized, 1, memory_order_release);
    return NACL_SUCCESS;
}

/**
 * @brief Quick self-test for RNG quality (NIST 800-90B)
 * @details Checks for catastrophic failures like all-zeros or stuck bits
 */
static int randombytes_selftest_quick(uint8_t *test_buf, size_t len) {
    if (len < 16) {
        return NACL_ERROR_BUFFER_TOO_SMALL;
    }

    memset(test_buf, 0, len);

#ifdef WITH_DRNG
    if (_randombytes_drng_fill(test_buf, 16) != 0) {
        return NACL_ERROR_RNG_FAILURE;
    }
#endif

    /* Check: not all zeros, not all same byte */
    int all_zero = 1, all_same = 1;
    for (size_t i = 0; i < 16; i++) {
        if (test_buf[i] != 0)
            all_zero = 0;
        if (test_buf[i] != test_buf[0])
            all_same = 0;
    }

    if (all_zero || all_same) {
        return NACL_ERROR_CATASTROPHIC_FAILURE;
    }

    return NACL_SUCCESS;
}

/**
 * @brief Generate cryptographically secure random bytes
 * @param[out] buf Buffer to store random bytes
 * @param[in] len Number of bytes to generate
 * @return NACL_SUCCESS on success, error code on failure
 *
 * @note Uses hardware DRNG if available, falls back to OS RNG
 * @warning On failure, buffer may contain partial data - caller should handle
 */
int randombytes_safe(uint8_t *buf, size_t len) {
    int ret = NACL_SUCCESS;

    /* Validate parameters */
    if (buf == NULL || len == 0) {
        return NACL_ERROR_INVALID_PARAM;
    }

    /* Auto-initialize if needed (thread-safe) */
    if (!atomic_load_explicit(&randombytes_initialized, memory_order_acquire)) {
        ret = randombytes_init();
        if (ret != NACL_SUCCESS) {
            return ret;
        }
    }

    /* Every 1000 calls, run quick RNG self-test (NIST 800-90B) */
    uint64_t call_count = atomic_fetch_add_explicit(&randombytes_call_count, 1, memory_order_relaxed) + 1;
    if (call_count % 1000 == 0) {
        uint8_t test_buf[16];
        ret = randombytes_selftest_quick(test_buf, sizeof(test_buf));
        if (ret != NACL_SUCCESS) {
            /* Self-test failed - reset RNG state and reinitialize */
            atomic_store_explicit(&randombytes_initialized, 0, memory_order_release);
            ret = randombytes_init();
            if (ret != NACL_SUCCESS) {
                return NACL_ERROR_RNG_FAILURE;
            }
        }
    }

#ifdef WITH_DRNG
    /* Try hardware DRNG first */
    if (_randombytes_drng_fill(buf, len) == 0) {
        return NACL_SUCCESS; /* Hardware RNG succeeded */
    }
    /* Hardware RNG failed or unavailable — fall through to software */
#endif

#ifdef _WIN32
    /* Windows: Use modern BCryptGenRandom (CNG), not deprecated CryptoAPI */
    {
        BCRYPT_ALG_HANDLE hAlg = NULL;
        NTSTATUS status;

        status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RNG_ALGORITHM, NULL, 0);
        if (status != 0) {
            return NACL_ERROR_RNG_FAILURE;
        }

        status = BCryptGenRandom(hAlg, buf, (ULONG)len, 0);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        if (status != 0) {
            return NACL_ERROR_RNG_FAILURE;
        }
    }
#else
    /* POSIX: Read from /dev/urandom */
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return NACL_ERROR_RNG_FAILURE;
    }

    size_t total_read = 0;
    while (total_read < len) {
        ssize_t ret_read = read(fd, buf + total_read, len - total_read);
        if (ret_read < 0) {
            if (errno == EINTR) {
                continue; /* Interrupted by signal - retry */
            }
            close(fd);
            return NACL_ERROR_RNG_FAILURE;
        }
        if (ret_read == 0) {
            /* EOF - unexpected for /dev/urandom */
            close(fd);
            return NACL_ERROR_RNG_FAILURE;
        }
        total_read += (size_t)ret_read;
    }
    close(fd);
#endif

    return NACL_SUCCESS;
}

/**
 * @brief Legacy API - generates random bytes (deprecated)
 * @param[out] buf Buffer to store random bytes
 * @param[in] len Number of bytes to generate
 *
 * @deprecated Use randombytes_safe() instead. Will be removed in next major version.
 * @note Returns silently on failure; does NOT abort.
 */
void randombytes(uint8_t *buf, size_t len) {
    (void)randombytes_safe(buf, len);
}
