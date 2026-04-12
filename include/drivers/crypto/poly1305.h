/*
 * Poly1305 Implementation Interface
 */

#ifndef IMPL_POLY1305_H
#define IMPL_POLY1305_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int poly1305_soft(u8 *out, const u8 *m, u64 n, const u8 *k);

#ifdef HAVE_SSE2_IMPL
int poly1305_sse2(u8 *out, const u8 *m, u64 n, const u8 *k);
#endif

#ifdef HAVE_NEON_IMPL
int poly1305_neon(u8 *out, const u8 *m, u64 n, const u8 *k);
#endif

#ifdef __cplusplus
}
#endif

#endif /* IMPL_POLY1305_H */
