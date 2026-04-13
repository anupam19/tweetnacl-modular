/*
 * FIPS Self-Test Public Interface
 * 
 * Implements FIPS 140-3 Section 9 requirements:
 *   - Power-Up Self-Tests (Section 9.3.1)
 *   - Conditional Self-Tests (Section 9.3.2)
 *   - Continuous RNG Test (NIST SP 800-90B)
 * 
 * Standards Compliance:
 *   - FIPS 140-3: Security Requirements for Cryptographic Modules
 *   - FIPS 186-5: Digital Signature Standard (Ed25519)
 *   - FIPS 180-4: Secure Hash Standard (SHA-512)
 *   - NIST SP 800-90B: Entropy Sources
 *   - NIST SP 800-88: Media Sanitization (Zeroization)
 */

#ifndef FIPS_SELFTEST_H
#define FIPS_SELFTEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FIPS module state enumeration
 * 
 * Tracks the operational state of the FIPS module through its lifecycle.
 * Module must reach FIPS_STATE_APPROVED before cryptographic operations
 * can be performed in FIPS mode.
 */
typedef enum {
    FIPS_STATE_INITIAL = 0,    /**< Module uninitialized */
    FIPS_STATE_SELFTEST = 1,   /**< Self-test in progress */
    FIPS_STATE_ERROR = 2,      /**< Self-test failed, module disabled */
    FIPS_STATE_APPROVED = 3    /**< Module approved and operational */
} fips_state_t;

/**
 * @brief Execute FIPS 140-3 Power-On Self-Test
 * 
 * Performs all required startup tests per FIPS 140-3 Section 9.3:
 *   1. Software Load Test (integrity verification)
 *   2. Known Answer Tests (KAT) for SHA-512, Ed25519, X25519
 *   3. Pairwise Consistency Tests for Ed25519, X25519
 *   4. Continuous RNG Test (CRNGT)
 * 
 * This function MUST be called during system initialization before
 * any cryptographic operations are performed in FIPS mode.
 * 
 * @return 0 on success, -1 on failure
 * @post Global fips_state set to FIPS_STATE_APPROVED on success
 * @post Global fips_state set to FIPS_STATE_ERROR on failure
 * @warning All subsequent crypto operations should check nacl_get_fips_state()
 * @note Must be called at least once before using crypto functions in FIPS mode
 */
int fips_power_on_selftest(void);

/**
 * @brief Execute conditional self-test for key generation
 * 
 * Performs pairwise consistency tests every 100 key generation operations
 * as required by FIPS 140-3 Section 9.3.2.
 * 
 * This function should be called after each key generation operation.
 * The actual test is only performed every FIPS_CONDITIONAL_INTERVAL calls.
 * 
 * @return 0 on success or if test not due, -1 on test failure
 * @post fips_state set to FIPS_STATE_ERROR on failure
 * @note Caller should stop cryptographic operations if -1 is returned
 */
int fips_conditional_selftest(void);

/**
 * @brief Check if FIPS mode is enabled at compile time
 * 
 * @return 1 if FIPS_MODE is defined, 0 otherwise
 * @note Runtime FIPS state should be checked with nacl_get_fips_state()
 */
int nacl_fips_mode_enabled(void);

/**
 * @brief Get string describing supported FIPS standards
 * 
 * @return Constant string listing FIPS/NIST standards implemented
 */
const char *nacl_fips_version(void);

/**
 * @brief Get current FIPS module state
 * 
 * @return Current fips_state_t value indicating module status
 * @note Should be checked before cryptographic operations in FIPS mode
 */
fips_state_t nacl_get_fips_state(void);

/**
 * @brief Get human-readable error description for FIPS state
 * 
 * @param state FIPS state to describe
 * @return Constant string describing the state
 */
const char* nacl_get_fips_error_string(fips_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* FIPS_SELFTEST_H */
