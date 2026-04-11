/*
 * TweetNaCl Salsa20 Core Module Header
 * Internal header for Salsa20 stream cipher primitives
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 */

#ifndef TWEETNACL_SALSA20_H
#define TWEETNACL_SALSA20_H

#include "internal/tweetnacl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Salsa20 core function
 * @param out Output buffer (64 bytes)
 * @param in Input buffer (16 bytes)
 * @param k Key buffer (32 bytes)
 * @param c Constants buffer (16 bytes)
 * @return 0 on success
 */
int crypto_core_salsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c);

/**
 * HSalsa20 core function (hash variant)
 * @param out Output buffer (32 bytes)
 * @param in Input buffer (16 bytes)
 * @param k Key buffer (32 bytes)
 * @param c Constants buffer (16 bytes)
 * @return 0 on success
 */
int crypto_core_hsalsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c);

/**
 * Salsa20 stream cipher
 * @param c Output ciphertext
 * @param d Data length
 * @param n Nonce (8 bytes)
 * @param k Key (32 bytes)
 * @return 0 on success
 */
int crypto_stream_salsa20(u8 *c, u64 d, const u8 *n, const u8 *k);

/**
 * Salsa20 stream cipher with XOR
 * @param c Output ciphertext
 * @param m Input message (can be NULL for keystream only)
 * @param b Length in bytes
 * @param n Nonce (8 bytes)
 * @param k Key (32 bytes)
 * @return 0 on success
 */
int crypto_stream_salsa20_xor(u8 *c, const u8 *m, u64 b, const u8 *n, const u8 *k);

/**
 * XSalsa20 stream cipher (extended nonce)
 * @param c Output ciphertext
 * @param d Data length
 * @param n Nonce (24 bytes)
 * @param k Key (32 bytes)
 * @return 0 on success
 */
int crypto_stream(u8 *c, u64 d, const u8 *n, const u8 *k);

/**
 * XSalsa20 stream cipher with XOR
 * @param c Output ciphertext
 * @param m Input message (can be NULL for keystream only)
 * @param d Length in bytes
 * @param n Nonce (24 bytes)
 * @param k Key (32 bytes)
 * @return 0 on success
 */
int crypto_stream_xor(u8 *c, const u8 *m, u64 d, const u8 *n, const u8 *k);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_SALSA20_H */
