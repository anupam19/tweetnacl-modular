/*
 * Salsa20 Implementation Interface
 */

#ifndef IMPL_SALSA20_H
#define IMPL_SALSA20_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Software implementation (always available) */
int salsa20_soft(u8 *out, const u8 *in, const u8 *k, const u8 *c);
int hsalsa20_soft(u8 *out, const u8 *in, const u8 *k, const u8 *c);

/* SSE2 implementations (if available) */
#ifdef HAVE_SSE2_IMPL
int salsa20_sse2(u8 *out, const u8 *in, const u8 *k, const u8 *c);
int hsalsa20_sse2(u8 *out, const u8 *in, const u8 *k, const u8 *c);
#endif

/* AVX2 implementations (if available) */
#ifdef HAVE_AVX2_IMPL
int salsa20_avx2(u8 *out, const u8 *in, const u8 *k, const u8 *c);
int hsalsa20_avx2(u8 *out, const u8 *in, const u8 *k, const u8 *c);
#endif

/* NEON implementations (if available) */
#ifdef HAVE_NEON_IMPL
int salsa20_neon(u8 *out, const u8 *in, const u8 *k, const u8 *c);
int hsalsa20_neon(u8 *out, const u8 *in, const u8 *k, const u8 *c);
#endif

/* SVE implementations (if available) */
#ifdef HAVE_SVE_IMPL
int salsa20_sve(u8 *out, const u8 *in, const u8 *k, const u8 *c);
int hsalsa20_sve(u8 *out, const u8 *in, const u8 *k, const u8 *c);
#endif

#ifdef __cplusplus
}
#endif

#endif /* IMPL_SALSA20_H */
