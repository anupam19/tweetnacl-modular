/*
 * Curve25519 Implementation Interface
 */

#ifndef IMPL_CURVE25519_H
#define IMPL_CURVE25519_H

#include "../tweetnacl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int curve25519_soft_scalarmult(u8 *q, const u8 *n, const u8 *p);
int curve25519_soft_scalarmult_base(u8 *q, const u8 *n);

#ifdef HAVE_SSE2_IMPL
int curve25519_sse2_scalarmult_base(u8 *q, const u8 *n);
#endif

#ifdef HAVE_NEON_IMPL
int curve25519_neon_scalarmult_base(u8 *q, const u8 *n);
#endif

#ifdef __cplusplus
}
#endif

#endif /* IMPL_CURVE25519_H */
