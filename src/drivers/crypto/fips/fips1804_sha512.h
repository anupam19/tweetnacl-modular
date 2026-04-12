/*
 * FIPS 180-4 SHA-512 Compliance Extensions
 * Includes SHA-512/256 and SHA-512/224 (FIPS 180-4 Section 6.7)
 */

#ifndef FIPS1804_SHA512_H
#define FIPS1804_SHA512_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SHA-512/256: SHA-512 with different IV, truncated to 256 bits */
int crypto_hash_sha512_256(uint8_t *out, const uint8_t *in, uint64_t inlen);

/* SHA-512/224: SHA-512 with different IV, truncated to 224 bits */
int crypto_hash_sha512_224(uint8_t *out, const uint8_t *in, uint64_t inlen);

#ifdef __cplusplus
}
#endif

#endif /* FIPS1804_SHA512_H */
