/*
 * Known Answer Tests (KATs) for TweetNaCl-Modular
 * NIST CAVP test vectors for all cryptographic primitives
 *
 * Implements NIST SP 800-193 Section 3.2 power-on self-tests
 * and NIST CAVP (Cryptographic Algorithm Validation Program) vectors.
 */

#ifndef KAT_RUNNER_H
#define KAT_RUNNER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Power-on self-test: run all KATs, returns 0 on success, -1 on failure */
int nacl_selftest_all(void);

/* Individual primitive self-tests */
int nacl_selftest_sha512(void);
int nacl_selftest_poly1305(void);
int nacl_selftest_curve25519(void);
int nacl_selftest_ed25519(void);
int nacl_selftest_secretbox(void);
int nacl_selftest_box(void);
int nacl_selftest_scalarmult(void);
int nacl_selftest_sign(void);

/* Library integrity check (NIST 800-193 Section 3.3) */
int nacl_integrity_check(void);

/* Key generation validation */
int nacl_keypair_validate(const uint8_t *pk, const uint8_t *sk);

#ifdef __cplusplus
}
#endif

#endif /* KAT_RUNNER_H */
