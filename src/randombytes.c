/*
 * Random Bytes Generation Implementation
 * Provides cryptographically secure random number generation
 *
 * Priority order:
 *   1. Hardware DRNG (RDSEED/RDRAND/ARM RNDR) — if WITH_DRNG
 *   2. BCryptGenRandom (Windows CNG)
 *   3. /dev/urandom (POSIX fallback)
 *
 * CERT C Compliance:
 * - MEM35-C: Allocate sufficient memory for an object
 * - ARR30-C: Do not form or use out-of-bounds pointers
 */

#include "randombytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* DRNG internal helpers (from randombytes_drng.c) */
#ifdef WITH_DRNG
extern int _randombytes_drng_fill(uint8_t *buf, size_t len);
extern int _randombytes_drng_impl_is_software(void);
#endif

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

void randombytes(uint8_t *buf, size_t len) {
    int failed = 0;

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
