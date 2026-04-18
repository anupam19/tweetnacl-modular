/*
 * Curve25519 Driver - Operations Interface
 * Delegates to main TweetNaCl implementations
 */

#include "core/types.h"
#include "drivers/crypto/curve25519.h"
#include "tweetnacl/tweetnacl.h"

/* Curve25519 scalar multiplication */
int curve25519_scalarmult(uint8_t *out, const uint8_t *scalar, const uint8_t *point) {
    if (!out || !scalar || !point)
        return -1;
    return crypto_scalarmult(out, scalar, point);
}

/* Curve25519 scalar multiplication with base point */
int curve25519_scalarmult_base(uint8_t *out, const uint8_t *scalar) {
    if (!out || !scalar)
        return -1;
    return crypto_scalarmult_base(out, scalar);
}
