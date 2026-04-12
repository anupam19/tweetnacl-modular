/*
 * Salsa20 Core Driver
 * Provides the core Salsa20/HSalsa20 implementation interface
 */

#ifndef SALSA20_H
#define SALSA20_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int crypto_core_salsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c);
int crypto_core_hsalsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c);

#ifdef __cplusplus
}
#endif

#endif /* SALSA20_H */
