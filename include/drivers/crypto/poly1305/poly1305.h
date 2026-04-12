/*
 * Poly1305 Driver Interface
 */

#ifndef POLY1305_H
#define POLY1305_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Poly1305 one-time authenticator */
int crypto_onetimeauth(u8 *out, const u8 *m, u64 n, const u8 *k);

/* Poly1305 verification */
int crypto_onetimeauth_verify(const u8 *h, const u8 *m, u64 n, const u8 *k);

#ifdef __cplusplus
}
#endif

#endif /* POLY1305_H */
