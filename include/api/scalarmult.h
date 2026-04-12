/* Scalar Multiplication API */
#ifndef TWEETNACL_SCALARMULT_H
#define TWEETNACL_SCALARMULT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define crypto_scalarmult_BYTES 32
#define crypto_scalarmult_SCALARBYTES 32
int crypto_scalarmult(uint8_t *q, const uint8_t *n, const uint8_t *p);
int crypto_scalarmult_base(uint8_t *q, const uint8_t *n);
#ifdef __cplusplus
}
#endif
#endif
