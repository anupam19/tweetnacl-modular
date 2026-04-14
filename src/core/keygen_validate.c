/*
 * Key Generation Validation
 *
 * Verifies that generated keypairs are structurally valid.
 * Catches faulty RNG or implementation bugs during key generation.
 */

#include "core/secure_mem.h"
#include "scalarmult/scalarmult.h"
#include "tweetnacl/tweetnacl.h"

/**
 * Validate an Ed25519 keypair
 * Recomputes public key from secret key and compares
 * @param pk Public key (32 bytes)
 * @param sk Secret key (64 bytes, includes public key in upper half)
 * @return 0 if valid, -1 if invalid
 */
int nacl_keypair_validate(const uint8_t *pk, const uint8_t *sk) {
    if (pk == NULL || sk == NULL)
        return -1;

    /* Recompute public key from the seed in the secret key */
    uint8_t test_pk[32];
    crypto_scalarmult_base(test_pk, sk);

    return secure_memcmp(test_pk, pk, 32) == 0 ? 0 : -1;
}

/**
 * Generate a validated Ed25519 keypair
 * Generates keys and validates them, retrying if validation fails
 * @param pk Output public key (32 bytes)
 * @param sk Output secret key (64 bytes)
 * @return 0 on success, -1 if validation fails after retries
 */
int nacl_keypair_generate_validated(uint8_t *pk, uint8_t *sk) {
    if (pk == NULL || sk == NULL)
        return -1;

    int attempts = 0;
    const int max_attempts = 3;

    do {
        if (crypto_box_keypair(pk, sk) != 0)
            return -1;
        attempts++;
    } while (nacl_keypair_validate(pk, sk) != 0 && attempts < max_attempts);

    return (attempts < max_attempts) ? 0 : -1;
}
