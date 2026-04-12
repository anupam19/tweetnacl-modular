/*
 * SHA-512 Driver Interface
 */

#ifndef SHA512_H
#define SHA512_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SHA-512 hash function */
int crypto_hash(u8 *out, const u8 *m, u64 n);

/* SHA-512 hash blocks compression */
int crypto_hashblocks(u8 *x, const u8 *m, u64 n);

#ifdef __cplusplus
}
#endif

#endif /* SHA512_H */
