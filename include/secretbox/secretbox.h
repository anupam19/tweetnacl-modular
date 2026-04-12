/* Secret-key Authenticated Encryption API */
#ifndef TWEETNACL_SECRETBOX_H
#define TWEETNACL_SECRETBOX_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define crypto_secretbox_KEYBYTES 32
#define crypto_secretbox_NONCEBYTES 24
#define crypto_secretbox_ZEROBYTES 32
#define crypto_secretbox_BOXZEROBYTES 16
int crypto_secretbox(uint8_t *c, const uint8_t *m, uint64_t d,
                     const uint8_t *n, const uint8_t *k);
int crypto_secretbox_open(uint8_t *m, const uint8_t *c, uint64_t d,
                          const uint8_t *n, const uint8_t *k);
#ifdef __cplusplus
}
#endif
#endif
