/* Hash API */
#ifndef TWEETNACL_HASH_H
#define TWEETNACL_HASH_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define crypto_hash_BYTES 64
int crypto_hash(uint8_t *out, const uint8_t *m, uint64_t n);
#ifdef __cplusplus
}
#endif
#endif
