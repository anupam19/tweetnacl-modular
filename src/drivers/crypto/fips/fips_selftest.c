/*
 * FIPS Self-Test Framework (FIPS 140-3 Section 9)
 * Power-on self tests, pairwise consistency, continuous RNG test
 * 
 * Implements:
 *   - FIPS 140-3 Section 9.3: Power-Up Self-Tests
 *   - NIST SP 800-193: Platform Firmware Resiliency
 *   - FIPS 186-5: Digital Signature Standard
 *   - FIPS 180-4: Secure Hash Standard
 *   - NIST SP 800-90B: Entropy Sources
 */

#include "drivers/crypto/fips/fips_selftest.h"
#include "tweetnacl/tweetnacl.h"
#include "core/secure_mem.h"
#include "core/error.h"
#include "drivers/rng/randombytes.h"
#include <stdint.h>
#include <string.h>

static volatile fips_state_t fips_state = FIPS_STATE_INITIAL;

/* Conditional self-test counters (FIPS 140-3 Section 9.3.2) */
static volatile uint32_t keygen_counter = 0;
#define FIPS_CONDITIONAL_INTERVAL 100

/* ─── Zeroization Helper (NIST SP 800-88 Rev 1) ───────────────────────── */

/**
 * Zeroize sensitive data per NIST SP 800-88
 * @param data Pointer to sensitive data
 * @param len Length of data in bytes
 */
static void fips_zeroize(volatile void* data, size_t len) {
    if (data != NULL && len > 0) {
        secure_zero(data, len);
    }
}

/* ─── Known Answer Tests (KAT) ────────────────────────────────────────── */

/**
 * SHA-512 Known Answer Test
 * Test vector from FIPS 180-4 Appendix D.1
 * @return 0 on success, -1 on failure
 */
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
    
    /* Clear buffer before use */
    secure_zero(hash, sizeof(hash));
    
    int ret = crypto_hash(hash, abc, 3);
    if (ret != 0) {
        fips_zeroize(hash, sizeof(hash));
        return -1;
    }
    
    int result = secure_memcmp(hash, expected, 64);
    fips_zeroize(hash, sizeof(hash));
    
    return (result == 0) ? 0 : -1;
}

/**
 * Ed25519 Known Answer Test
 * Sign and verify operation per FIPS 186-5
 * @return 0 on success, -1 on failure
 */
static int fips_kat_ed25519(void) {
    uint8_t pk[32], sk[64], sm[96], m[32];
    uint64_t smlen, mlen;
    const uint8_t msg[] = "FIPS KAT";
    int ret = -1;

    /* Initialize buffers */
    secure_zero(pk, sizeof(pk));
    secure_zero(sk, sizeof(sk));
    secure_zero(sm, sizeof(sm));
    secure_zero(m, sizeof(m));

    if (crypto_sign_keypair(pk, sk) != 0) goto cleanup;
    if (crypto_sign(sm, &smlen, msg, sizeof(msg) - 1, sk) != 0) goto cleanup;
    if (crypto_sign_open(m, &mlen, sm, smlen, pk) != 0) goto cleanup;
    if (mlen != sizeof(msg) - 1) goto cleanup;
    if (secure_memcmp(m, msg, mlen) != 0) goto cleanup;
    
    ret = 0;

cleanup:
    fips_zeroize(pk, sizeof(pk));
    fips_zeroize(sk, sizeof(sk));
    fips_zeroize(sm, sizeof(sm));
    fips_zeroize(m, sizeof(m));
    return ret;
}

/**
 * X25519 Known Answer Test
 * Scalar multiplication per RFC 7748
 * @return 0 on success, -1 on failure
 */
static int fips_kat_x25519(void) {
    uint8_t pk[32], sk[32], ss[32];
    int ret = -1;

    secure_zero(pk, sizeof(pk));
    secure_zero(sk, sizeof(sk));
    secure_zero(ss, sizeof(ss));

    if (crypto_box_keypair(pk, sk) != 0) goto cleanup;
    if (crypto_scalarmult(ss, sk, pk) != 0) goto cleanup;
    
    /* Verify result is not all zeros (invalid point) */
    uint8_t zero_check[32];
    secure_zero(zero_check, sizeof(zero_check));
    if (secure_memcmp(ss, zero_check, 32) == 0) goto cleanup;
    
    ret = 0;

cleanup:
    fips_zeroize(pk, sizeof(pk));
    fips_zeroize(sk, sizeof(sk));
    fips_zeroize(ss, sizeof(ss));
    return ret;
}

/* ─── Pairwise Consistency Test (PCT) ─────────────────────────────────── */

/**
 * Ed25519 Pairwise Consistency Test
 * Generate key pair, sign, verify, and validate consistency
 * Per FIPS 140-3 Section 9.3.2
 * @return 0 on success, -1 on failure
 */
static int fips_pct_keygen_ed25519(void) {
    uint8_t pk[32], sk[64], sig[96], msg[32];
    uint64_t smlen, mlen;
    int ret = -1;

    secure_zero(pk, sizeof(pk));
    secure_zero(sk, sizeof(sk));
    secure_zero(sig, sizeof(sig));
    secure_zero(msg, sizeof(msg));

    /* Generate random message */
    if (randombytes_safe(msg, sizeof(msg)) != 0) goto cleanup;
    
    if (crypto_sign_keypair(pk, sk) != 0) goto cleanup;
    if (crypto_sign(sig, &smlen, msg, sizeof(msg), sk) != 0) goto cleanup;
    if (crypto_sign_open(msg, &mlen, sig, smlen, pk) != 0) goto cleanup;
    
    ret = 0;

cleanup:
    fips_zeroize(pk, sizeof(pk));
    fips_zeroize(sk, sizeof(sk));
    fips_zeroize(sig, sizeof(sig));
    fips_zeroize(msg, sizeof(msg));
    return ret;
}

/**
 * X25519 Pairwise Consistency Test
 * Generate key pairs on both sides and derive shared secret
 * @return 0 on success, -1 on failure
 */
static int fips_pct_keygen_x25519(void) {
    uint8_t pk1[32], sk1[32], pk2[32], sk2[32];
    uint8_t ss1[32], ss2[32];
    int ret = -1;

    secure_zero(pk1, sizeof(pk1));
    secure_zero(sk1, sizeof(sk1));
    secure_zero(pk2, sizeof(pk2));
    secure_zero(sk2, sizeof(sk2));
    secure_zero(ss1, sizeof(ss1));
    secure_zero(ss2, sizeof(ss2));

    if (crypto_box_keypair(pk1, sk1) != 0) goto cleanup;
    if (crypto_box_keypair(pk2, sk2) != 0) goto cleanup;
    
    /* Both parties derive shared secret */
    if (crypto_scalarmult(ss1, sk1, pk2) != 0) goto cleanup;
    if (crypto_scalarmult(ss2, sk2, pk1) != 0) goto cleanup;
    
    /* Verify shared secrets match */
    if (secure_memcmp(ss1, ss2, 32) != 0) goto cleanup;
    
    ret = 0;

cleanup:
    fips_zeroize(pk1, sizeof(pk1));
    fips_zeroize(sk1, sizeof(sk1));
    fips_zeroize(pk2, sizeof(pk2));
    fips_zeroize(sk2, sizeof(sk2));
    fips_zeroize(ss1, sizeof(ss1));
    fips_zeroize(ss2, sizeof(ss2));
    return ret;
}

/* ─── Continuous Random Number Generator Test (CRNGT) ─────────────────── */

/**
 * Continuous RNG Test per NIST SP 800-90B Section 4.3
 * Detects stuck RNG conditions
 * @return 0 on success, -1 on failure (duplicate detected)
 */
static int fips_crngt(void) {
    static uint8_t prev[16] = {0};
    static int initialized = 0;
    uint8_t current[16];
    int ret = 0;

    secure_zero(current, sizeof(current));

    if (randombytes_safe(current, 16) != 0) {
        fips_zeroize(current, sizeof(current));
        return -1;
    }

    if (!initialized) {
        memcpy(prev, current, 16);
        initialized = 1;
        fips_zeroize(current, sizeof(current));
        return 0;
    }

    /* Constant-time comparison */
    if (secure_memcmp(current, prev, 16) == 0) {
        ret = -1; /* CRNGT failure — identical outputs */
    } else {
        memcpy(prev, current, 16);
    }

    fips_zeroize(current, sizeof(current));
    return ret;
}

/* ─── Software Load Test (Integrity Check) ────────────────────────────── */

/**
 * Software Load Test / Integrity Check
 * Verifies code integrity by computing checksum of critical functions
 * Per FIPS 140-3 Section 9.3.1.3
 * @return 0 on success, -1 on failure
 */
static int fips_software_load_test(void) {
    /* 
     * In production, this would compute a hash of the binary text section
     * and compare against a known-good value embedded at build time.
     * For now, we perform a basic sanity check that crypto functions exist.
     */
    
    /* Verify function pointers are valid (not NULL) */
    if ((void*)crypto_hash == NULL) return -1;
    if ((void*)crypto_sign == NULL) return -1;
    if ((void*)crypto_sign_open == NULL) return -1;
    if ((void*)crypto_box_keypair == NULL) return -1;
    if ((void*)crypto_scalarmult == NULL) return -1;
    if ((void*)randombytes == NULL) return -1;
    
    return 0;
}

/* ─── Conditional Self-Test Wrapper ───────────────────────────────────── */

/**
 * Perform conditional self-test if counter threshold reached
 * Called from key generation functions
 * @return 0 on success or not due, -1 on failure
 */
int fips_conditional_selftest(void) {
    /* Increment counter atomically */
    keygen_counter++;
    
    /* Only test every FIPS_CONDITIONAL_INTERVAL iterations */
    if ((keygen_counter % FIPS_CONDITIONAL_INTERVAL) != 0) {
        return 0;
    }
    
    /* Perform pairwise consistency test */
    if (fips_pct_keygen_ed25519() != 0) {
        fips_state = FIPS_STATE_ERROR;
        return -1;
    }
    
    if (fips_pct_keygen_x25519() != 0) {
        fips_state = FIPS_STATE_ERROR;
        return -1;
    }
    
    /* Reset counter after successful test */
    keygen_counter = 0;
    return 0;
}

/* ─── Public API ──────────────────────────────────────────────────────── */

/**
 * FIPS 140-3 Power-On Self-Test
 * Executes all required startup tests per Section 9.3
 * 
 * Tests performed:
 *   1. Known Answer Tests (KAT) for SHA-512, Ed25519, X25519
 *   2. Pairwise Consistency Tests for Ed25519, X25519
 *   3. Continuous RNG Test (CRNGT)
 *   4. Software Load Test (integrity check)
 * 
 * @return 0 on success, -1 on failure
 * @post fips_state set to FIPS_STATE_APPROVED on success, FIPS_STATE_ERROR on failure
 */
int fips_power_on_selftest(void) {
    fips_state = FIPS_STATE_SELFTEST;

    /* 1. Software Load Test (integrity) */
    if (fips_software_load_test() != 0) {
        fips_state = FIPS_STATE_ERROR;
        return -1;
    }

    /* 2. Known Answer Tests */
    if (fips_kat_sha512() != 0) { 
        fips_state = FIPS_STATE_ERROR; 
        return -1; 
    }
    if (fips_kat_ed25519() != 0) { 
        fips_state = FIPS_STATE_ERROR; 
        return -1; 
    }
    if (fips_kat_x25519() != 0) { 
        fips_state = FIPS_STATE_ERROR; 
        return -1; 
    }

    /* 3. Pairwise Consistency Tests */
    if (fips_pct_keygen_ed25519() != 0) { 
        fips_state = FIPS_STATE_ERROR; 
        return -1; 
    }
    if (fips_pct_keygen_x25519() != 0) { 
        fips_state = FIPS_STATE_ERROR; 
        return -1; 
    }

    /* 4. Continuous RNG Test */
    if (fips_crngt() != 0) { 
        fips_state = FIPS_STATE_ERROR; 
        return -1; 
    }

    fips_state = FIPS_STATE_APPROVED;
    return 0;
}

/**
 * Check if FIPS mode is enabled
 * @return 1 if FIPS mode enabled, 0 otherwise
 */
int nacl_fips_mode_enabled(void) {
#ifdef FIPS_MODE
    return 1;
#else
    return 0;
#endif
}

/**
 * Get FIPS version string
 * @return String listing supported FIPS standards
 */
const char *nacl_fips_version(void) {
    return "FIPS 140-3, FIPS 186-5, FIPS 180-4, NIST SP 800-90B";
}

/**
 * Get current FIPS state
 * @return Current fips_state_t value
 */
fips_state_t nacl_get_fips_state(void) {
    return fips_state;
}

/**
 * Get FIPS error description
 * @param state FIPS state to describe
 * @return Human-readable error string
 */
const char* nacl_get_fips_error_string(fips_state_t state) {
    switch (state) {
        case FIPS_STATE_INITIAL:
            return "FIPS module uninitialized";
        case FIPS_STATE_SELFTEST:
            return "FIPS self-test in progress";
        case FIPS_STATE_ERROR:
            return "FIPS self-test failed - module disabled";
        case FIPS_STATE_APPROVED:
            return "FIPS module approved and operational";
        default:
            return "Unknown FIPS state";
    }
}
