/* Digital Signatures API */
#ifndef TWEETNACL_SIGN_H
#define TWEETNACL_SIGN_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define crypto_sign_BYTES 64
#define crypto_sign_PUBLICKEYBYTES 32
#define crypto_sign_SECRETKEYBYTES 64
int crypto_sign_keypair(uint8_t *pk, uint8_t *sk);
int crypto_sign(uint8_t *sm, uint64_t *smlen, const uint8_t *m, uint64_t n, const uint8_t *sk);
int crypto_sign_open(uint8_t *m, uint64_t *mlen, const uint8_t *sm, uint64_t n, const uint8_t *pk);
#ifdef __cplusplus
}
#endif
#endif
