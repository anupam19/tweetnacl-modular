/*
 * TweetNaCl HMAC-SHA512-256 Module Header
 * Internal header for HMAC-SHA512-256 authentication
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 */

#ifndef TWEETNACL_HMACSHA512256_H
#define TWEETNACL_HMACSHA512256_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * HMAC-SHA512-256 authentication
 * @param out Output MAC (32 bytes)
 * @param m Input message
 * @param n Message length
 * @param k Key (32 bytes)
 * @return 0 on success
 */
int crypto_auth_hmacsha512256(u8 *out, const u8 *m, u64 n, const u8 *k);

/**
 * HMAC-SHA512-256 verification
 * @param h Expected MAC (32 bytes)
 * @param m Input message
 * @param n Message length
 * @param k Key (32 bytes)
 * @return 0 if valid, -1 if invalid
 */
int crypto_auth_hmacsha512256_verify(const u8 *h, const u8 *m, u64 n, const u8 *k);

/**
 * Generic auth interface (maps to HMAC-SHA512-256)
 */
#define crypto_auth crypto_auth_hmacsha512256
#define crypto_auth_verify crypto_auth_hmacsha512256_verify

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_HMACSHA512256_H */
