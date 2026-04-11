/*
 * Random Bytes Generation Implementation
 * Provides cryptographically secure random number generation
 * 
 * CERT C Compliance:
 * - MEM35-C: Allocate sufficient memory for an object
 * - ARR30-C: Do not form or use out-of-bounds pointers
 */

#include "randombytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

void randombytes(uint8_t *buf, size_t len) {
    int failed = 0;
    
#ifdef _WIN32
    static HCRYPTPROV prov = 0;
    if (prov == 0) {
        if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, 0)) {
            failed = 1;
        }
    }
    if (!failed && !CryptGenRandom(prov, (DWORD)len, buf)) {
        failed = 1;
    }
#else
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
