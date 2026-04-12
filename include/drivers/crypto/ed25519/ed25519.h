/*
 * Ed25519 Driver Interface
 */

#ifndef ED25519_H
#define ED25519_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Ed25519 key pair generation */
int crypto_sign_keypair(u8 *pk, u8 *sk);

/* Ed25519 signing */
int crypto_sign(u8 *sm, u64 *smlen, const u8 *m, u64 n, const u8 *sk);

/* Ed25519 signature verification */
int crypto_sign_open(u8 *m, u64 *mlen, const u8 *sm, u64 n, const u8 *pk);

#ifdef __cplusplus
}
#endif

#endif /* ED25519_H */
