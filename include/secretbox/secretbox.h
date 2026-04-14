/* Secret-key Authenticated Encryption API */
#ifndef TWEETNACL_SECRETBOX_H
#define TWEETNACL_SECRETBOX_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int crypto_secretbox(uint8_t *c, const uint8_t *m, uint64_t d, const uint8_t *n, const uint8_t *k);
int crypto_secretbox_open(uint8_t *m, const uint8_t *c, uint64_t d, const uint8_t *n,
                          const uint8_t *k);
#ifdef __cplusplus
}
#endif
#endif
