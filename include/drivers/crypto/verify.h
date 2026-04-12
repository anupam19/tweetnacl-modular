/*
 * TweetNaCl Verify Module Header
 * Internal header for constant-time memory comparison
 * 
 * CERT C Compliance:
 * - No forward declarations
 * - Self-contained header
 * - Constant-time operations to prevent timing attacks
 */

#ifndef TWEETNACL_VERIFY_H
#define TWEETNACL_VERIFY_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Constant-time 16-byte comparison
 * @param x First buffer (16 bytes)
 * @param y Second buffer (16 bytes)
 * @return 0 if equal, -1 if different
 */
int crypto_verify_16(const u8 *x, const u8 *y);

/**
 * Constant-time 32-byte comparison
 * @param x First buffer (32 bytes)
 * @param y Second buffer (32 bytes)
 * @return 0 if equal, -1 if different
 */
int crypto_verify_32(const u8 *x, const u8 *y);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_VERIFY_H */
