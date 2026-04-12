/*
 * Hardware DRNG (Digital Random Number Generator) Implementation
 * Provides hardware-accelerated random number generation with fallback
 *
 * Supported Instructions:
 *   x86_64:  RDRAND, RDSEED
 *   ARM64:   RNDR (ARMv8.5+)
 *
 * Fallback Strategy (priority order):
 *   1. RDSEED (best for seeding)
 *   2. RDRAND (good for general random data)
 *   3. ARM RNDR
 *   4. BCryptGenRandom (Windows CNG)
 *   5. /dev/urandom (POSIX fallback)
 *
 * CERT C Compliance:
 *   - MEM35-C: Allocate sufficient memory for an object
 *   - ARR30-C: Do not form or use out-of-bounds pointers
 */

#include "api/randombytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ─── Implementation tracking ───────────────────────────────────────────── */
static enum {
    RNG_NONE,
    RNG_RDSEED,
    RNG_RDRAND,
    RNG_ARM_RNDR,
    RNG_BCRYPT,
    RNG_URANDOM
} rng_impl = RNG_NONE;

/* ─── x86 RDRAND / RDSEED ───────────────────────────────────────────────── */
#if defined(HAS_RDSEED) || defined(HAS_RDRAND)

#include <immintrin.h>

#ifdef HAS_RDSEED
static int rdseed64_retry(uint64_t *val) {
    /* RDSEED may need multiple retries if carry flag is clear */
    int retries = 10;
    while (retries-- > 0) {
        if (_rdseed64_step(val)) return 1;
    }
    return 0;
}
#endif

#ifdef HAS_RDRAND
static int rdrand64_retry(uint64_t *val) {
    /* RDRAND may need multiple retries if carry flag is clear */
    int retries = 10;
    while (retries-- > 0) {
        if (_rdrand64_step(val)) return 1;
    }
    return 0;
}
#endif

#endif /* HAS_RDSEED || HAS_RDRAND */

/* ─── ARM RNDR ───────────────────────────────────────────────────────────── */
#ifdef HAS_ARM_RNG

#include <stdint.h>

static int arm_rndr(uint64_t *val) {
    uint64_t rnd;
    /* MRS from S3_3_C2_C4_0 = RNDR register on ARMv8.5+ */
    asm volatile("mrs %0, s3_3_c2_c4_0" : "=r"(rnd));
    *val = rnd;
    return 1;
}

#endif /* HAS_ARM_RNG */

/* ─── Runtime CPU feature detection (x86) ────────────────────────────────── */
#if defined(__x86_64__) || defined(__i386__)

static int cpuid_has_rdrand(void) {
#ifdef HAS_RDRAND
    uint32_t eax, ebx, ecx, edx;
    /* CPUID leaf 1, ECX bit 30 = RDRAND */
    __asm__ __volatile__("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1));
    return (ecx >> 30) & 1;
#else
    return 0;
#endif
}

static int cpuid_has_rdseed(void) {
#ifdef HAS_RDSEED
    uint32_t eax, ebx, ecx, edx;
    /* CPUID leaf 7, EBX bit 18 = RDSEED */
    __asm__ __volatile__("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(7), "c"(0));
    return (ebx >> 18) & 1;
#else
    return 0;
#endif
}

#endif /* x86 */

/* ─── Forward declarations for internal helpers ──────────────────────────── */
int _randombytes_drng_fill(uint8_t *buf, size_t len);
int _randombytes_drng_impl_is_software(void);

/* ─── Init: detect best available RNG at runtime ─────────────────────────── */
static void drng_init(void) {
#if defined(__x86_64__) || defined(__i386__)
    /* Prefer RDSEED over RDRAND (RDSEED is designed for seeding) */
    if (cpuid_has_rdseed()) {
        rng_impl = RNG_RDSEED;
        return;
    }
    if (cpuid_has_rdrand()) {
        rng_impl = RNG_RDRAND;
        return;
    }
#endif

#ifdef HAS_ARM_RNG
    /* ARMv8.5+ RNDR — always succeeds on supported hardware */
    {
        uint64_t test_val;
        if (arm_rndr(&test_val)) {
            rng_impl = RNG_ARM_RNDR;
            return;
        }
    }
#endif

    /* No hardware RNG available — will use software fallback */
}

/* ─── Fill buffer using hardware RNG ─────────────────────────────────────── */
static int drng_fill(uint8_t *buf, size_t len) {
    size_t offset = 0;

    while (offset + 8 <= len) {
        uint64_t val = 0;
        int ok = 0;

        switch (rng_impl) {
        case RNG_RDSEED:
#ifdef HAS_RDSEED
            ok = rdseed64_retry(&val);
#else
            return -1;
#endif
            break;
        case RNG_RDRAND:
#ifdef HAS_RDRAND
            ok = rdrand64_retry(&val);
#else
            return -1;
#endif
            break;
        case RNG_ARM_RNDR:
#ifdef HAS_ARM_RNG
            ok = arm_rndr(&val);
#else
            return -1;
#endif
            break;
        case RNG_NONE:
        case RNG_BCRYPT:
        case RNG_URANDOM:
        default:
            return -1;
        }

        if (!ok) return -1; /* Hardware failure — fall back to software */

        memcpy(buf + offset, &val, 8);
        offset += 8;
    }

    /* Handle remaining bytes (< 8) */
    if (offset < len) {
        uint64_t val = 0;
        int ok = 0;

        switch (rng_impl) {
        case RNG_RDSEED:
#ifdef HAS_RDSEED
            ok = rdseed64_retry(&val);
#else
            return -1;
#endif
            break;
        case RNG_RDRAND:
#ifdef HAS_RDRAND
            ok = rdrand64_retry(&val);
#else
            return -1;
#endif
            break;
        case RNG_ARM_RNDR:
#ifdef HAS_ARM_RNG
            ok = arm_rndr(&val);
#else
            return -1;
#endif
            break;
        case RNG_NONE:
        case RNG_BCRYPT:
        case RNG_URANDOM:
        default:
            return -1;
        }

        if (!ok) return -1;

        size_t remaining = len - offset;
        memcpy(buf + offset, &val, remaining);
    }

    return 0;
}

/* ─── Public API ─────────────────────────────────────────────────────────── */

int randombytes_drng_available(void) {
    if (rng_impl == RNG_NONE) {
        drng_init();
    }
    return (rng_impl != RNG_NONE) ? 1 : 0;
}

const char *randombytes_implementation_name(void) {
    if (rng_impl == RNG_NONE) {
        drng_init();
    }
    switch (rng_impl) {
    case RNG_NONE:     return "none";
    case RNG_RDSEED:   return "RDSEED";
    case RNG_RDRAND:   return "RDRAND";
    case RNG_ARM_RNDR: return "ARM_RNDR";
    case RNG_BCRYPT:   return "BCryptGenRandom";
    case RNG_URANDOM:  return "/dev/urandom";
    default:           return "unknown";
    }
}

/* Internal accessor for randombytes.c to use hardware RNG */
int _randombytes_drng_fill(uint8_t *buf, size_t len) {
    if (rng_impl == RNG_NONE) {
        drng_init();
    }
    if (rng_impl == RNG_NONE) {
        return -1; /* No hardware RNG available */
    }
    return drng_fill(buf, len);
}

int _randombytes_drng_impl_is_software(void) {
    if (rng_impl == RNG_NONE) {
        drng_init();
    }
    return (rng_impl == RNG_NONE) ? 1 : 0;
}
