/*
 * Public-Key Authenticated Encryption API (crypto_box)
 */

#ifndef TWEETNACL_BOX_H
#define TWEETNACL_BOX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int crypto_box_keypair(uint8_t *pk, uint8_t *sk);
int crypto_box(uint8_t *c, const uint8_t *m, uint64_t d, const uint8_t *n, const uint8_t *pk,
               const uint8_t *sk);
int crypto_box_open(uint8_t *m, const uint8_t *c, uint64_t d, const uint8_t *n, const uint8_t *pk,
                    const uint8_t *sk);

/* Precomputation functions */
int crypto_box_beforenm(uint8_t *k, const uint8_t *pk, const uint8_t *sk);
int crypto_box_afternm(uint8_t *c, const uint8_t *m, uint64_t d, const uint8_t *n,
                       const uint8_t *k);
int crypto_box_open_afternm(uint8_t *m, const uint8_t *c, uint64_t d, const uint8_t *n,
                            const uint8_t *k);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_BOX_H */
