/*
 * Public-Key Authenticated Encryption API (crypto_box)
 *
 * This header provides the high-level crypto_box API.
 * Implementation is in src/api/box.c (uses crypto_ops internally)
 */

#ifndef TWEETNACL_BOX_H
#define TWEETNACL_BOX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Re-export box constants from core types */
#define crypto_box_PUBLICKEYBYTES 32
#define crypto_box_SECRETKEYBYTES 32
#define crypto_box_NONCEBYTES 24
#define crypto_box_ZEROBYTES 32
#define crypto_box_BOXZEROBYTES 16

int crypto_box_keypair(uint8_t *pk, uint8_t *sk);
int crypto_box(uint8_t *c, const uint8_t *m, uint64_t d,
               const uint8_t *n, const uint8_t *pk, const uint8_t *sk);
int crypto_box_open(uint8_t *m, const uint8_t *c, uint64_t d,
                    const uint8_t *n, const uint8_t *pk, const uint8_t *sk);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_BOX_H */
