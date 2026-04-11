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
#include "internal/tweetnacl_types.h"
#include "internal/tweetnacl_verify.h"
#include "internal/tweetnacl_salsa20.h"
#include "internal/tweetnacl_poly1305.h"
#include "internal/tweetnacl_sha512.h"
#include "internal/tweetnacl_curve25519.h"
#include "internal/tweetnacl_ed25519.h"
#include "internal/tweetnacl_hmacsha512256.h"
#include "internal/tweetnacl_secretbox.h"
#include "internal/tweetnacl_box.h"

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

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_H */
