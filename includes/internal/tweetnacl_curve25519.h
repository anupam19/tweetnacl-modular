/*
 * TweetNaCl Curve25519 Module Header
 * Internal header for Curve25519 elliptic curve operations
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 * - Constant-time operations
 */

#ifndef TWEETNACL_CURVE25519_H
#define TWEETNACL_CURVE25519_H

#include "internal/tweetnacl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Curve25519 scalar multiplication
 * Computes q = n * p where n is a scalar and p is a point
 * @param q Output point (32 bytes)
 * @param n Scalar (32 bytes)
 * @param p Input point (32 bytes)
 * @return 0 on success
 */
int crypto_scalarmult_curve25519(u8 *q, const u8 *n, const u8 *p);

/**
 * Curve25519 scalar multiplication with base point
 * Computes q = n * B where B is the base point
 * @param q Output point (32 bytes)
 * @param n Scalar (32 bytes)
 * @return 0 on success
 */
int crypto_scalarmult_curve25519_base(u8 *q, const u8 *n);

/**
 * Generic scalarmult interface (maps to Curve25519)
 */
#define crypto_scalarmult crypto_scalarmult_curve25519
#define crypto_scalarmult_base crypto_scalarmult_curve25519_base

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_CURVE25519_H */
