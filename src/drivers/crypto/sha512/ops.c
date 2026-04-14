/*
 * SHA-512 Driver - Operations Interface
 * Delegates to main TweetNaCl implementations
 */

#include "core/types.h"
#include "drivers/crypto/sha512.h"
#include "tweetnacl/tweetnacl.h"

/* SHA-512 hash computation */
int sha512_hash(uint8_t *out, const uint8_t *in, size_t in_len) {
    if (!out || !in) return -1;
    return crypto_hash(out, in, in_len);
}

/* SHA-512 hash blocks compression */
int sha512_hashblocks(uint8_t *x, const uint8_t *m, size_t m_len) {
    if (!x || !m) return -1;
    return crypto_hashblocks(x, m, m_len);
}
