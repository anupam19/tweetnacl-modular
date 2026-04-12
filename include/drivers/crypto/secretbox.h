/*
 * TweetNaCl SecretBox Module Header
 * Internal header for XSalsa20-Poly1305 symmetric encryption
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 */

#ifndef TWEETNACL_SECRETBOX_H
#define TWEETNACL_SECRETBOX_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * XSalsa20-Poly1305 symmetric encryption
 * @param c Output ciphertext (includes 16-byte MAC prefix)
 * @param m Input message
 * @param d Message length (must be >= ZEROBYTES)
 * @param n Nonce (24 bytes)
 * @param k Key (32 bytes)
 * @return 0 on success, -1 if message too short
 */
int crypto_secretbox_xsalsa20poly1305(u8 *c, const u8 *m, u64 d, const u8 *n, const u8 *k);

/**
 * XSalsa20-Poly1305 decryption
 * @param m Output plaintext
 * @param c Input ciphertext (includes 16-byte MAC prefix)
 * @param d Ciphertext length (must be >= ZEROBYTES)
 * @param n Nonce (24 bytes)
 * @param k Key (32 bytes)
 * @return 0 on success, -1 if verification fails or too short
 */
int crypto_secretbox_xsalsa20poly1305_open(u8 *m, const u8 *c, u64 d, const u8 *n, const u8 *k);

/**
 * Generic secretbox interface (maps to XSalsa20-Poly1305)
 */
#define crypto_secretbox crypto_secretbox_xsalsa20poly1305
#define crypto_secretbox_open crypto_secretbox_xsalsa20poly1305_open

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_SECRETBOX_H */
