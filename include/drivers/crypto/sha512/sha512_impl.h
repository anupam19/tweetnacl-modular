/*
 * TweetNaCl SHA-512 Hash Module Header
 * Internal header for SHA-512 hash function
 *
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 */

#ifndef TWEETNACL_SHA512_H
#define TWEETNACL_SHA512_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SHA-512 hash blocks compression function
 * @param x State buffer (64 bytes)
 * @param m Message blocks
 * @param n Number of bytes to process
 * @return Remaining bytes not processed
 */
int crypto_hashblocks_sha512(u8 *x, const u8 *m, u64 n);

/**
 * SHA-256 hash blocks compression function
 * @param x State buffer (32 bytes)
 * @param m Message blocks
 * @param n Number of bytes to process
 * @return Remaining bytes not processed
 */
int crypto_hashblocks_sha256(u8 *x, const u8 *m, u64 n);

/**
 * SHA-512 hash function
 * @param out Output hash (64 bytes)
 * @param m Input message
 * @param n Message length
 * @return 0 on success
 */
int crypto_hash_sha512(u8 *out, const u8 *m, u64 n);

/**
 * SHA-256 hash function
 * @param out Output hash (32 bytes)
 * @param m Input message
 * @param n Message length
 * @return 0 on success
 */
int crypto_hash_sha256(u8 *out, const u8 *m, u64 n);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_SHA512_H */
