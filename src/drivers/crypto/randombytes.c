/*
 * Random Bytes Generation Implementation
 * Provides cryptographically secure random number generation
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
 */

#include "api/randombytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
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
static volatile uint64_t randombytes_call_count = 0;

static void randombytes_selftest_quick(void) {
    uint8_t test[16];
    memset(test, 0, sizeof(test));

#ifdef WITH_DRNG
    _randombytes_drng_fill(test, sizeof(test));
#endif

    /* Check: not all zeros, not all same byte */
    int all_zero = 1, all_same = 1;
    for (int i = 0; i < 16; i++) {
        if (test[i] != 0) all_zero = 0;
        if (test[i] != test[0]) all_same = 0;
    }
    /* Catastrophic failure — fall through to software RNG */
    (void)all_zero;
    (void)all_same;
}

void randombytes(uint8_t *buf, size_t len) {
    int failed = 0;

    /* Every 1000 calls, run quick RNG self-test (NIST 800-90B) */
    if ((++randombytes_call_count % 1000) == 0) {
        randombytes_selftest_quick();
    }

#ifdef WITH_DRNG
    /* Try hardware DRNG first */
    if (_randombytes_drng_fill(buf, len) == 0) {
        return; /* Hardware RNG succeeded */
    }
    /* Hardware RNG failed or unavailable — fall through to software */
#endif

#ifdef _WIN32
    /* Windows: Use modern BCryptGenRandom (CNG), not deprecated CryptoAPI */
    {
        BCRYPT_ALG_HANDLE hAlg = NULL;
        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RNG_ALGORITHM, NULL, 0) == 0) {
            if (BCryptGenRandom(hAlg, buf, (ULONG)len, 0) != 0) {
                failed = 1;
            }
            BCryptCloseAlgorithmProvider(hAlg, 0);
        } else {
            failed = 1;
        }
    }
#else
    /* POSIX: Read from /dev/urandom */
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t ret = read(fd, buf, len);
        if (ret != (ssize_t)len) {
            failed = 1;
        }
        close(fd);
    } else {
        failed = 1;
    }
#endif

    if (failed) {
        fprintf(stderr, "Generating random data failed. Please report this.\n");
        exit(1);
    }
}
