/*
 * FIPS 186-5 Ed25519 Context Support
 * Ed25519ctx context string handling per FIPS 186-5 Section 7
 */

#ifndef FIPS1865_CTX_H
#define FIPS1865_CTX_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FIPS 186-5 DOM2 prefix */
#define FIPS1865_DOM2_PREFIX "SigEd25519 no Ed25519 collisions"

/* Ed25519ctx with context string */
int crypto_sign_ed25519ctx(uint8_t *sm, uint64_t *smlen,
                           const uint8_t *m, uint64_t mlen,
                           const uint8_t *sk,
                           const char *ctx, size_t ctx_len);

int crypto_sign_ed25519ctx_open(uint8_t *m, uint64_t *mlen,
                                const uint8_t *sm, uint64_t smlen,
                                const uint8_t *pk,
                                const char *ctx, size_t ctx_len);

/* Ed25519ph (prehash) per FIPS 186-5 Section 7.6 */
int crypto_sign_ed25519ph(uint8_t *sm, uint64_t *smlen,
                          const uint8_t *m, uint64_t mlen,
                          const uint8_t *sk);

int crypto_sign_ed25519ph_open(uint8_t *m, uint64_t *mlen,
                               const uint8_t *sm, uint64_t smlen,
                               const uint8_t *pk);

#ifdef __cplusplus
}
#endif

#endif /* FIPS1865_CTX_H */
