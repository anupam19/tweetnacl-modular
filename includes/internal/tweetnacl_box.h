/*
 * TweetNaCl Box Module Header
 * Internal header for Curve25519-XSalsa20-Poly1305 public-key encryption
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 */

#ifndef TWEETNACL_BOX_H
#define TWEETNACL_BOX_H

#include "internal/tweetnacl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Curve25519-XSalsa20-Poly1305 keypair generation
 * @param pk Output public key (32 bytes)
 * @param sk Output secret key (32 bytes)
 * @return 0 on success
 */
int crypto_box_curve25519xsalsa20poly1305_keypair(u8 *pk, u8 *sk);

/**
 * Curve25519 precomputation for shared key
 * @param k Output shared key (32 bytes)
 * @param pk Peer's public key (32 bytes)
 * @param sk Our secret key (32 bytes)
 * @return 0 on success
 */
int crypto_box_curve25519xsalsa20poly1305_beforenm(u8 *k, const u8 *pk, const u8 *sk);

/**
 * Curve25519-XSalsa20-Poly1305 encryption with precomputed key
 * @param c Output ciphertext
 * @param m Input message
 * @param d Message length (must be >= ZEROBYTES)
 * @param n Nonce (24 bytes)
 * @param k Precomputed shared key (32 bytes)
 * @return 0 on success
 */
int crypto_box_curve25519xsalsa20poly1305_afternm(u8 *c, const u8 *m, u64 d, const u8 *n, const u8 *k);

/**
 * Curve25519-XSalsa20-Poly1305 decryption with precomputed key
 * @param m Output plaintext
 * @param c Input ciphertext
 * @param d Ciphertext length (must be >= ZEROBYTES)
 * @param n Nonce (24 bytes)
 * @param k Precomputed shared key (32 bytes)
 * @return 0 on success, -1 if verification fails
 */
int crypto_box_curve25519xsalsa20poly1305_open_afternm(u8 *m, const u8 *c, u64 d, const u8 *n, const u8 *k);

/**
 * Curve25519-XSalsa20-Poly1305 encryption (full)
 * @param c Output ciphertext
 * @param m Input message
 * @param d Message length (must be >= ZEROBYTES)
 * @param n Nonce (24 bytes)
 * @param pk Recipient's public key (32 bytes)
 * @param sk Sender's secret key (32 bytes)
 * @return 0 on success
 */
int crypto_box_curve25519xsalsa20poly1305(u8 *c, const u8 *m, u64 d, const u8 *n, const u8 *pk, const u8 *sk);

/**
 * Curve25519-XSalsa20-Poly1305 decryption (full)
 * @param m Output plaintext
 * @param c Input ciphertext
 * @param d Ciphertext length (must be >= ZEROBYTES)
 * @param n Nonce (24 bytes)
 * @param pk Sender's public key (32 bytes)
 * @param sk Recipient's secret key (32 bytes)
 * @return 0 on success, -1 if verification fails
 */
int crypto_box_curve25519xsalsa20poly1305_open(u8 *m, const u8 *c, u64 d, const u8 *n, const u8 *pk, const u8 *sk);

/**
 * Generic box interface (maps to Curve25519-XSalsa20-Poly1305)
 */
#define crypto_box crypto_box_curve25519xsalsa20poly1305
#define crypto_box_open crypto_box_curve25519xsalsa20poly1305_open
#define crypto_box_keypair crypto_box_curve25519xsalsa20poly1305_keypair
#define crypto_box_beforenm crypto_box_curve25519xsalsa20poly1305_beforenm
#define crypto_box_afternm crypto_box_curve25519xsalsa20poly1305_afternm
#define crypto_box_open_afternm crypto_box_curve25519xsalsa20poly1305_open_afternm

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_BOX_H */
