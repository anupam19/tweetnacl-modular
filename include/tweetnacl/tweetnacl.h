/*
 * TweetNaCl Public API Header
 * 
 * A minimal, auditable cryptographic library refactored for:
 * - CERT C secure coding compliance
 * - Modular design with no forward declarations
 * - Multi-architecture support (x86, ARM, RISC-V)
 * - Post-Quantum Cryptography integration
 * 
 * Usage: Include this single header for all crypto operations.
 */

#ifndef TWEETNACL_H
#define TWEETNACL_H

#include <stdint.h>
#include <stddef.h>

/* Import internal module headers */
#include "core/types.h"
#include "drivers/crypto/verify.h"
#include "drivers/crypto/salsa20/salsa20_impl.h"
#include "drivers/crypto/poly1305/poly1305_impl.h"
#include "drivers/crypto/sha512/sha512_impl.h"
#include "drivers/crypto/curve25519/curve25519_impl.h"
#include "drivers/crypto/ed25519.h"
#include "drivers/crypto/hmacsha512256.h"
#include "secretbox/secretbox.h"
#include "box/box.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * ALGORITHM IDENTIFIERS AND VERSION STRINGS
 * ============================================================ */

/* Auth: HMAC-SHA512-256 */
#define crypto_auth_PRIMITIVE "hmacsha512256"
#define crypto_auth_BYTES 32U
#define crypto_auth_KEYBYTES 32U
#define crypto_auth_IMPLEMENTATION "crypto_auth/hmacsha512256/tweet"
#define crypto_auth_VERSION "-"

/* Box: Curve25519-XSalsa20-Poly1305 */
#define crypto_box_PRIMITIVE "curve25519xsalsa20poly1305"
#define crypto_box_PUBLICKEYBYTES 32U
#define crypto_box_SECRETKEYBYTES 32U
#define crypto_box_BEFORENMBYTES 32U
#define crypto_box_NONCEBYTES 24U
#define crypto_box_ZEROBYTES 32U
#define crypto_box_BOXZEROBYTES 16U
#define crypto_box_IMPLEMENTATION "crypto_box/curve25519xsalsa20poly1305/tweet"
#define crypto_box_VERSION "-"

/* Core: Salsa20 */
#define crypto_core_PRIMITIVE "salsa20"
#define crypto_core_OUTPUTBYTES 64U
#define crypto_core_INPUTBYTES 16U
#define crypto_core_KEYBYTES 32U
#define crypto_core_CONSTBYTES 16U
#define crypto_core_IMPLEMENTATION "crypto_core/salsa20/tweet"
#define crypto_core_VERSION "-"

/* Core: HSalsa20 */
#define crypto_core_hsalsa20_OUTPUTBYTES 32U
#define crypto_core_hsalsa20_INPUTBYTES 16U
#define crypto_core_hsalsa20_KEYBYTES 32U
#define crypto_core_hsalsa20_CONSTBYTES 16U
#define crypto_core_hsalsa20_IMPLEMENTATION "crypto_core/hsalsa20/tweet"
#define crypto_core_hsalsa20_VERSION "-"

/* Hash: SHA-512 */
#define crypto_hash_PRIMITIVE "sha512"
#define crypto_hash_BYTES 64U
#define crypto_hash_IMPLEMENTATION "crypto_hash/sha512/tweet"
#define crypto_hash_VERSION "-"

/* Hash: SHA-256 */
#define crypto_hash_sha256_BYTES 32U
#define crypto_hash_sha256_IMPLEMENTATION "crypto_hash/sha256/tweet"
#define crypto_hash_sha256_VERSION "-"

/* Hashblocks */
#define crypto_hashblocks_PRIMITIVE "sha512"
#define crypto_hashblocks_STATEBYTES 64U
#define crypto_hashblocks_BLOCKBYTES 128U
#define crypto_hashblocks_IMPLEMENTATION "crypto_hashblocks/sha512/tweet"
#define crypto_hashblocks_VERSION "-"

/* One-time auth: Poly1305 */
#define crypto_onetimeauth_PRIMITIVE "poly1305"
#define crypto_onetimeauth_BYTES 16U
#define crypto_onetimeauth_KEYBYTES 32U
#define crypto_onetimeauth_IMPLEMENTATION "crypto_onetimeauth/poly1305/tweet"
#define crypto_onetimeauth_VERSION "-"

/* Scalarmult: Curve25519 */
#define crypto_scalarmult_PRIMITIVE "curve25519"
#define crypto_scalarmult_BYTES 32U
#define crypto_scalarmult_SCALARBYTES 32U
#define crypto_scalarmult_IMPLEMENTATION "crypto_scalarmult/curve25519/tweet"
#define crypto_scalarmult_VERSION "-"

/* Secretbox: XSalsa20-Poly1305 */
#define crypto_secretbox_PRIMITIVE "xsalsa20poly1305"
#define crypto_secretbox_KEYBYTES 32U
#define crypto_secretbox_NONCEBYTES 24U
#define crypto_secretbox_ZEROBYTES 32U
#define crypto_secretbox_BOXZEROBYTES 16U
#define crypto_secretbox_IMPLEMENTATION "crypto_secretbox/xsalsa20poly1305/tweet"
#define crypto_secretbox_VERSION "-"

/* Sign: Ed25519 */
#define crypto_sign_PRIMITIVE "ed25519"
#define crypto_sign_BYTES 64U
#define crypto_sign_PUBLICKEYBYTES 32U
#define crypto_sign_SECRETKEYBYTES 64U
#define crypto_sign_IMPLEMENTATION "crypto_sign/ed25519/tweet"
#define crypto_sign_VERSION "-"

/* ============================================================
 * PUBLIC FUNCTION DECLARATIONS
 * ============================================================ */

/**
 * SHA-512 cryptographic hash function
 * @param out Output buffer (64 bytes)
 * @param m Input message
 * @param n Message length
 * @return 0 on success
 */
int crypto_hash(u8 *out, const u8 *m, u64 n);

/**
 * SHA-512 hash blocks compression function (internal, exposed for advanced use)
 * @param x State buffer (64 bytes)
 * @param m Message blocks
 * @param n Number of bytes to process
 * @return Remaining bytes not processed
 */
int crypto_hashblocks(u8 *x, const u8 *m, u64 n);

/* ============================================================
 * SELF-TEST AND INTEGRITY (NIST SP 800-193)
 * ============================================================ */

/**
 * Run all Known Answer Tests (KATs) for cryptographic primitives
 * Tests SHA-512, Poly1305, Curve25519, Ed25519, SecretBox, Box
 * @return 0 if all tests pass, -1 if any test fails
 */
int nacl_selftest_all(void);

/**
 * Verify library integrity (checks code section hash)
 * May be a no-op on unsupported platforms
 * @return 0 if integrity verified, -1 if tampering detected
 */
int nacl_integrity_check(void);

/**
 * Validate an Ed25519 keypair by recomputing public key
 * @param pk Public key (32 bytes)
 * @param sk Secret key (64 bytes)
 * @return 0 if valid, -1 if invalid
 */
int nacl_keypair_validate(const uint8_t *pk, const uint8_t *sk);

/**
 * Generate a validated Ed25519 keypair with retry logic
 * @param pk Output public key (32 bytes)
 * @param sk Output secret key (64 bytes)
 * @return 0 on success, -1 if validation fails after retries
 */
int nacl_keypair_generate_validated(uint8_t *pk, uint8_t *sk);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_H */
