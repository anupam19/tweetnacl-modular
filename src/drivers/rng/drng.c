/*
 * Hardware DRNG (Digital Random Number Generator) Implementation
 * Provides hardware-accelerated random number generation with fallback
 *
 * Inspired by Intel DRNG samples (Intel Corp, 2014, BSD-3)
 * Adapted for TweetNaCl-Modular architecture
 *
 * Supported Instructions:
 *   x86_64:  RDRAND, RDSEED (via intrinsics or raw asm)
 *   ARM64:   RNDR (ARMv8.5+)
 *
 * Fallback Strategy (priority order):
 *   1. RDSEED (best for seeding — NIST SP 800-90B)
 *   2. RDRAND (good for general random data)
 *   3. ARM RNDR
 *   4. BCryptGenRandom (Windows CNG)
 *   5. /dev/urandom (POSIX fallback)
 *
 * RDRAND_RETRIES: The recommended number of retries is 10 (per Intel),
 * based on a binomial probability argument — the odds of 10 failures
 * in a row are astronomically small.
 *
 * CERT C Compliance:
 *   - MEM35-C: Allocate sufficient memory for an object
 *   - ARR30-C: Do not form or use out-of-bounds pointers
 */

#include "drivers/rng/randombytes.h"
#include "drivers/rng/drng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define RDRAND_RETRIES 10

/* ─── Implementation tracking ───────────────────────────────────────────── */
static enum {
    RNG_NONE,
    RNG_RDSEED,
    RNG_RDRAND,
    RNG_ARM_RNDR,
    RNG_BCRYPT,
    RNG_URANDOM
} rng_impl = RNG_NONE;

/* ─── x86 CPUID detection ──────────────────────────────────────────────── */
#if defined(__x86_64__) || defined(__i386__)

typedef struct {
    uint32_t eax, ebx, ecx, edx;
} cpuid_regs_t;

static void x86_cpuid(cpuid_regs_t *info, uint32_t leaf, uint32_t subleaf) {
#ifdef __i386__
    __asm__ __volatile__("mov %%ebx,%0; cpuid; xchg %%ebx,%0"
        : "=r"(info->ebx), "=a"(info->eax), "=c"(info->ecx), "=d"(info->edx)
        : "a"(leaf), "c"(subleaf));
#else
    __asm__ __volatile__("cpuid"
        : "=a"(info->eax), "=b"(info->ebx), "=c"(info->ecx), "=d"(info->edx)
        : "a"(leaf), "c"(subleaf));
#endif
}

static int x86_has_rdrand(void) {
    cpuid_regs_t regs;
    x86_cpuid(&regs, 1, 0);
    return (regs.ecx >> 30) & 1;
}

static int x86_has_rdseed(void) {
    cpuid_regs_t regs;
    x86_cpuid(&regs, 7, 0);
    return (regs.ebx >> 18) & 1;
}

/* ─── RDRAND primitives ───────────────────────────────────────────────── */
#ifdef HAS_RDRAND
#include <immintrin.h>
static int rdrand64_step(uint64_t *val) { return _rdrand64_step(val); }
static int rdrand32_step(uint32_t *val) { return _rdrand32_step(val); }
#else
/* Raw assembly fallback for compilers without intrinsics */
static int rdrand64_step(uint64_t *val) {
    unsigned char ok;
#ifdef __x86_64__
    __asm__ __volatile__("rdrand %0; setc %1" : "=r"(*val), "=qm"(ok));
    return (int)ok;
#else
    /* 32-bit: two 32-bit reads */
    uint32_t lo, hi;
    __asm__ __volatile__("rdrand %0; setc %1" : "=r"(lo), "=qm"(ok) : : "edx");
    if (!ok) return 0;
    __asm__ __volatile__("rdrand %0; setc %1" : "=r"(hi), "=qm"(ok) : : "edx");
    *val = ((uint64_t)hi << 32) | lo;
    return (int)ok;
#endif
}
static int rdrand32_step(uint32_t *val) {
    unsigned char ok;
    __asm__ __volatile__("rdrand %0; setc %1" : "=r"(*val), "=qm"(ok) : : "edx");
    return (int)ok;
}
#endif /* HAS_RDRAND */

/* ─── RDSEED primitives ──────────────────────────────────────────────── */
#ifdef HAS_RDSEED
#include <immintrin.h>
static int rdseed64_step(uint64_t *val) { return _rdseed64_step(val); }
#else
/* Raw assembly fallback for compilers without intrinsics */
static int rdseed64_step(uint64_t *val) {
    unsigned char ok;
#ifdef __x86_64__
    __asm__ __volatile__("rdseed %0; setc %1" : "=r"(*val), "=qm"(ok));
    return (int)ok;
#else
    uint32_t lo, hi;
    __asm__ __volatile__("rdseed %0; setc %1" : "=r"(lo), "=qm"(ok) : : "edx");
    if (!ok) return 0;
    __asm__ __volatile__("rdseed %0; setc %1" : "=r"(hi), "=qm"(ok) : : "edx");
    *val = ((uint64_t)hi << 32) | lo;
    return (int)ok;
#endif
}
#endif /* HAS_RDSEED */

/* ─── Retry helpers ───────────────────────────────────────────────────── */
static int rdrand64_retry(uint64_t *val) {
    unsigned int count = 0;
    while (count <= RDRAND_RETRIES) {
        if (rdrand64_step(val)) return 1;
        ++count;
    }
    return 0;
}
static int rdseed64_retry(uint64_t *val) {
    unsigned int count = 0;
    while (count <= RDRAND_RETRIES) {
        if (rdseed64_step(val)) return 1;
        ++count;
    }
    return 0;
}
#endif /* __x86_64__ || __i386__ */

/* ─── ARM RNDR ──────────────────────────────────────────────────────────── */
#ifdef HAS_ARM_RNG
static int arm_rndr(uint64_t *val) {
    uint64_t rnd;
    __asm__ __volatile__("mrs %0, s3_3_c2_c4_0" : "=r"(rnd));
    *val = rnd;
    return 1;
}
#endif

/* ─── drng_fill: Fill buffer with hardware RNG ─────────────────────────── */
static int drng_fill(uint8_t *buf, size_t len) {
    size_t offset = 0;
#if defined(__x86_64__) || defined(__i386__)
    while (offset + 8 <= len) {
        uint64_t val;
        int ok = (rng_impl == RNG_RDSEED) ? rdseed64_retry(&val)
                                           : rdrand64_retry(&val);
        if (!ok) return -1;
        memcpy(buf + offset, &val, 8);
        offset += 8;
    }
    if (offset + 4 <= len) {
        uint32_t val;
        if (!rdrand32_step(&val)) return -1;
        memcpy(buf + offset, &val, 4);
        offset += 4;
    }
    if (offset < len) {
        uint32_t val;
        if (!rdrand32_step(&val)) return -1;
        memcpy(buf + offset, &val, len - offset);
    }
    return 0;
#elif defined(HAS_ARM_RNG)
    while (offset + 8 <= len) {
        uint64_t val;
        if (!arm_rndr(&val)) return -1;
        memcpy(buf + offset, &val, 8);
        offset += 8;
    }
    if (offset < len) {
        uint64_t val;
        if (!arm_rndr(&val)) return -1;
        memcpy(buf + offset, &val, len - offset);
    }
    return 0;
#else
    (void)buf; (void)len;
    return -1;
#endif
}

/* ─── Init: detect best available RNG at runtime ───────────────────────── */
static void drng_init(void) {
#if defined(__x86_64__) || defined(__i386__)
    if (x86_has_rdseed()) { rng_impl = RNG_RDSEED; return; }
    if (x86_has_rdrand()) { rng_impl = RNG_RDRAND; return; }
#endif
#ifdef HAS_ARM_RNG
    { uint64_t t; if (arm_rndr(&t)) { rng_impl = RNG_ARM_RNDR; return; } }
#endif
}

/* ─── Public API ───────────────────────────────────────────────────────── */
int randombytes_drng_available(void) {
    if (rng_impl == RNG_NONE) drng_init();
    return (rng_impl != RNG_NONE) ? 1 : 0;
}

int drng_get_drng_support(void) {
    if (rng_impl == RNG_NONE) drng_init();
    int f = DRNG_NO_SUPPORT;
    if (rng_impl == RNG_RDRAND) f |= DRNG_HAS_RDRAND;
    if (rng_impl == RNG_RDSEED) f |= DRNG_HAS_RDSEED;
    return f;
}

const char *randombytes_implementation_name(void) {
    if (rng_impl == RNG_NONE) drng_init();
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

int _randombytes_drng_fill(uint8_t *buf, size_t len) {
    if (rng_impl == RNG_NONE) drng_init();
    if (rng_impl == RNG_NONE) return -1;
    return drng_fill(buf, len);
}
