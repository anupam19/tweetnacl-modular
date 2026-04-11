/*
 * TweetNaCl Ed25519 Module Header
 * Internal header for Ed25519 digital signatures
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Minimal public API exposure
 */

#ifndef TWEETNACL_ED25519_H
#define TWEETNACL_ED25519_H

#include "internal/tweetnacl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Ed25519 keypair generation
 * @param pk Output public key (32 bytes)
 * @param sk Output secret key (64 bytes)
 * @return 0 on success
 */
int crypto_sign_ed25519_keypair(u8 *pk, u8 *sk);

/**
 * Ed25519 signature generation
 * @param sm Signed message (message + signature)
 * @param smlen Output length of signed message
 * @param m Input message
 * @param n Message length
 * @param sk Secret key (64 bytes)
 * @return 0 on success
 */
int crypto_sign_ed25519(u8 *sm, u64 *smlen, const u8 *m, u64 n, const u8 *sk);

/**
 * Ed25519 signature verification
 * @param m Output message (extracted from signed message)
 * @param mlen Output message length
 * @param sm Signed message
 * @param n Signed message length
 * @param pk Public key (32 bytes)
 * @return 0 if valid, -1 if invalid
 */
int crypto_sign_ed25519_open(u8 *m, u64 *mlen, const u8 *sm, u64 n, const u8 *pk);

/**
 * Generic sign interface (maps to Ed25519)
 */
#define crypto_sign crypto_sign_ed25519
#define crypto_sign_open crypto_sign_ed25519_open
#define crypto_sign_keypair crypto_sign_ed25519_keypair

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_ED25519_H */
