/*
 * SHA-512 Implementation Interface
 */

#ifndef IMPL_SHA512_H
#define IMPL_SHA512_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int sha512_soft(u8 *out, const u8 *m, u64 n);

#ifdef HAVE_SHANI_IMPL
/* SHA-NI accelerated SHA-512 */
int sha512_shani(u8 *out, const u8 *m, u64 n);
#endif

#ifdef HAVE_NEON_IMPL
int sha512_neon(u8 *out, const u8 *m, u64 n);
#endif

#ifdef __cplusplus
}
#endif

#endif /* IMPL_SHA512_H */
