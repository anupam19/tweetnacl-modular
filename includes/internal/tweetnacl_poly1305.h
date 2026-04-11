/*
 * TweetNaCl Poly1305 MAC Module Header
 * Internal header for Poly1305 one-time authenticator
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 */

#ifndef TWEETNACL_POLY1305_H
#define TWEETNACL_POLY1305_H

#include "internal/tweetnacl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Poly1305 one-time authenticator
 * @param out Output MAC (16 bytes)
 * @param m Input message
 * @param n Message length
 * @param k Key (32 bytes)
 * @return 0 on success
 */
int crypto_onetimeauth(u8 *out, const u8 *m, u64 n, const u8 *k);

/**
 * Poly1305 verification
 * @param h Expected MAC (16 bytes)
 * @param m Input message
 * @param n Message length
 * @param k Key (32 bytes)
 * @return 0 if valid, -1 if invalid
 */
int crypto_onetimeauth_verify(const u8 *h, const u8 *m, u64 n, const u8 *k);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_POLY1305_H */
