/*
 * Ed25519 Driver - Operations Interface
 * Delegates to main TweetNaCl implementations
 */

#include "core/types.h"
#include "drivers/crypto/ed25519.h"
#include "tweetnacl/tweetnacl.h"
#include <string.h>

/* Ed25519 signing */
int ed25519_sign(uint8_t *sig, unsigned long long *sig_len, const uint8_t *msg, size_t msg_len,
                 const uint8_t *sk) {
    if (!sig || !sig_len || !msg || !sk)
        return -1;
    return crypto_sign(sig, sig_len, msg, msg_len, sk);
}

/* Ed25519 verification */
int ed25519_verify(uint8_t *msg, unsigned long long *msg_len, const uint8_t *sm, size_t sm_len,
                   const uint8_t *pk) {
    if (!msg || !msg_len || !sm || !pk)
        return -1;
    return crypto_sign_open(msg, msg_len, sm, sm_len, pk);
}

/* Ed25519 keypair generation */
int ed25519_keypair(uint8_t *pk, uint8_t *sk) {
    if (!pk || !sk)
        return -1;
    return crypto_sign_keypair(pk, sk);
}
