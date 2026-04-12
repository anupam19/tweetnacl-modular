/*
 * Curve25519 Driver Interface
 */

#ifndef CURVE25519_H
#define CURVE25519_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Curve25519 scalar multiplication */
int crypto_scalarmult(u8 *q, const u8 *n, const u8 *p);

/* Curve25519 scalar multiplication with base point */
int crypto_scalarmult_base(u8 *q, const u8 *n);

#ifdef __cplusplus
}
#endif

#endif /* CURVE25519_H */
