/*
 * Poly1305 Driver - Operations Interface
 * Delegates to main TweetNaCl implementations
 */

#include "core/types.h"
#include "drivers/crypto/poly1305.h"
#include "tweetnacl/tweetnacl.h"

/* Poly1305 authentication tag generation */
int poly1305_auth(uint8_t *out, const uint8_t *in, size_t in_len, const uint8_t *key) {
    if (!out || !in || !key)
        return -1;
    return crypto_onetimeauth(out, in, in_len, key);
}

/* Poly1305 verification */
int poly1305_verify(const uint8_t *mac, const uint8_t *in, size_t in_len, const uint8_t *key) {
    if (!mac || !in || !key)
        return -1;
    return crypto_onetimeauth_verify(mac, in, in_len, key);
}
