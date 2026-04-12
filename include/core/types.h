/*
 * TweetNaCl Core Types and Constants Header
 * Internal types used across cryptographic primitives
 * 
 * CERT C Compliance:
 * - Uses explicit-width integer types
 * - No forward declarations
 * - Self-contained header
 */

#ifndef TWEETNACL_TYPES_H
#define TWEETNACL_TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Explicit-width integer types for portability */
typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t  i64;

/* Galois Field element for Curve25519 (16 limbs) */
typedef i64 gf[16];

/* Common buffer sizes */
enum {
    TWEETNACL_HASH_BYTES = 64,
    TWEETNACL_HASH_SHA256_BYTES = 32,
    TWEETNACL_AUTH_BYTES = 32,
    TWEETNACL_AUTH_KEYBYTES = 32,
    TWEETNACL_BOX_PUBLICKEYBYTES = 32,
    TWEETNACL_BOX_SECRETKEYBYTES = 32,
    TWEETNACL_BOX_BEFORENMBYTES = 32,
    TWEETNACL_BOX_NONCEBYTES = 24,
    TWEETNACL_BOX_ZEROBYTES = 32,
    TWEETNACL_BOX_BOXZEROBYTES = 16,
    TWEETNACL_SIGN_BYTES = 64,
    TWEETNACL_SIGN_PUBLICKEYBYTES = 32,
    TWEETNACL_SIGN_SECRETKEYBYTES = 64,
    TWEETNACL_SCALARMULT_BYTES = 32,
    TWEETNACL_SCALARMULT_SCALARBYTES = 32,
    TWEETNACL_SECRETBOX_KEYBYTES = 32,
    TWEETNACL_SECRETBOX_NONCEBYTES = 24,
    TWEETNACL_SECRETBOX_ZEROBYTES = 32,
    TWEETNACL_SECRETBOX_BOXZEROBYTES = 16,
    TWEETNACL_ONETIMEAUTH_BYTES = 16,
    TWEETNACL_ONETIMEAUTH_KEYBYTES = 32,
    TWEETNACL_CORE_OUTPUTBYTES = 64,
    TWEETNACL_CORE_INPUTBYTES = 16,
    TWEETNACL_CORE_KEYBYTES = 32,
    TWEETNACL_CORE_CONSTBYTES = 16,
    TWEETNACL_HASHBLOCKS_STATEBYTES = 64,
    TWEETNACL_HASHBLOCKS_BLOCKBYTES = 128
};

/* Constant-time comparison result */
#define TWEETNACL_SUCCESS 0
#define TWEETNACL_ERROR (-1)

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_TYPES_H */
