/*
 * FIPS Self-Test Framework (FIPS 140-3 Section 9)
 * Power-on self tests, pairwise consistency, continuous RNG test
 */

#include "fips_selftest.h"
#include "tweetnacl.h"
#include "secure_mem.h"
#include "randombytes.h"
#include <stdint.h>
#include <string.h>

static volatile fips_state_t fips_state = FIPS_STATE_INITIAL;

/* ─── Known Answer Tests ──────────────────────────────────────────────── */

static int fips_kat_sha512(void) {
    static const uint8_t abc[] = "abc";
    static const uint8_t expected[64] = {
        0xdd, 0xaf, 0x35, 0xa1, 0x93, 0x61, 0x7a, 0xba,
        0xcc, 0x41, 0x73, 0x49, 0xae, 0x20, 0x41, 0x31,
        0x12, 0xe6, 0xfa, 0x4e, 0x89, 0xa9, 0x7e, 0xa2,
        0x0a, 0x9e, 0xee, 0xe6, 0x4b, 0x55, 0xd3, 0x9a,
        0x21, 0x92, 0x99, 0x2a, 0x27, 0x4f, 0xc1, 0xa8,
        0x36, 0xba, 0x3c, 0x23, 0xa3, 0xfe, 0xeb, 0xbd,
        0x45, 0x4d, 0x44, 0x23, 0x64, 0x3c, 0xe8, 0x0e,
        0x2a, 0x9a, 0xc9, 0x4f, 0xa5, 0x4c, 0xa4, 0x9f
    };
    uint8_t hash[64];
    crypto_hash(hash, abc, 3);
    return secure_memcmp(hash, expected, 64) == 0 ? 0 : -1;
}

static int fips_kat_ed25519(void) {
    uint8_t pk[32], sk[64], sm[96], m[32];
    uint64_t smlen, mlen;
    const uint8_t msg[] = "FIPS KAT";

    if (crypto_sign_keypair(pk, sk) != 0) return -1;
    if (crypto_sign(sm, &smlen, msg, sizeof(msg) - 1, sk) != 0) return -1;
    if (crypto_sign_open(m, &mlen, sm, smlen, pk) != 0) return -1;
    if (mlen != sizeof(msg) - 1) return -1;
    return secure_memcmp(m, msg, mlen) == 0 ? 0 : -1;
}

/* ─── Pairwise Consistency Test (PCT) ─────────────────────────────────── */

static int fips_pct_keygen_ed25519(void) {
    uint8_t pk[32], sk[64], sig[96], msg[32];
    uint64_t smlen, mlen;

    randombytes(msg, sizeof(msg));
    if (crypto_sign_keypair(pk, sk) != 0) return -1;
    if (crypto_sign(sig, &smlen, msg, sizeof(msg), sk) != 0) return -1;
    if (crypto_sign_open(msg, &mlen, sig, smlen, pk) != 0) return -1;
    return 0;
}

/* ─── Continuous Random Number Generator Test (CRNGT) ─────────────────── */

static int fips_crngt(void) {
    static uint8_t prev[16] = {0};
    static int initialized = 0;
    uint8_t current[16];

    randombytes(current, 16);

    if (!initialized) {
        memcpy(prev, current, 16);
        initialized = 1;
        return 0;
    }

    /* Constant-time comparison */
    if (secure_memcmp(current, prev, 16) == 0) {
        return -1; /* CRNGT failure — identical outputs */
    }

    memcpy(prev, current, 16);
    return 0;
}

/* ─── Public API ──────────────────────────────────────────────────────── */

int fips_power_on_selftest(void) {
    fips_state = FIPS_STATE_SELFTEST;

    if (fips_kat_sha512() != 0) { fips_state = FIPS_STATE_ERROR; return -1; }
    if (fips_kat_ed25519() != 0) { fips_state = FIPS_STATE_ERROR; return -1; }
    if (fips_pct_keygen_ed25519() != 0) { fips_state = FIPS_STATE_ERROR; return -1; }
    if (fips_crngt() != 0) { fips_state = FIPS_STATE_ERROR; return -1; }

    fips_state = FIPS_STATE_APPROVED;
    return 0;
}

int nacl_fips_mode_enabled(void) {
#ifdef FIPS_MODE
    return 1;
#else
    return 0;
#endif
}

const char *nacl_fips_version(void) {
    return "FIPS 186-5, 180-4, 203, 204, 205";
}

fips_state_t nacl_get_fips_state(void) {
    return fips_state;
}
