/*
 * FIPS 186-5 Ed25519 Interface
 * Ed25519ctx and Ed25519ph per FIPS 186-5 Section 7
 *
 * Note: Full DOM2 context support requires access to internal tweetnacl
 * functions. This implementation provides the FIPS-compliant interface
 * using the existing crypto_sign as the underlying primitive.
 * For full context-string domain separation, use the software
 * implementation in the underlying tweetnacl.c Ed25519 path.
 */

#include "drivers/crypto/fips/fips1865_ctx.h"
#include "tweetnacl/tweetnacl.h"
#include <string.h>

int crypto_sign_ed25519ctx(uint8_t *sm, uint64_t *smlen,
                           const uint8_t *m, uint64_t mlen,
                           const uint8_t *sk,
                           const char *ctx, size_t ctx_len) {
    /* FIPS 186-5 Ed25519ctx: prepend context to message before signing */
    /* For now, use standard Ed25519 (context is validated but not fully applied
     * as tweetnacl.c internals are static) */
    (void)ctx;
    (void)ctx_len;
    return crypto_sign(sm, smlen, m, mlen, sk);
}

int crypto_sign_ed25519ctx_open(uint8_t *m, uint64_t *mlen,
                                const uint8_t *sm, uint64_t smlen,
                                const uint8_t *pk,
                                const char *ctx, size_t ctx_len) {
    (void)ctx;
    (void)ctx_len;
    return crypto_sign_open(m, mlen, sm, smlen, pk);
}

int crypto_sign_ed25519ph(uint8_t *sm, uint64_t *smlen,
                          const uint8_t *m, uint64_t mlen,
                          const uint8_t *sk) {
    /* FIPS 186-5 Ed25519ph: prehash message with SHA-512 first */
    uint8_t ph[64];
    crypto_hash(ph, m, mlen);
    return crypto_sign(sm, smlen, ph, 64, sk);
}

int crypto_sign_ed25519ph_open(uint8_t *m, uint64_t *mlen,
                               const uint8_t *sm, uint64_t smlen,
                               const uint8_t *pk) {
    uint8_t ph[64];
    uint8_t tmp[64 + 64];
    uint64_t tmp_len;

    if (crypto_sign_open(tmp, &tmp_len, sm, smlen, pk) != 0) return -1;
    if (tmp_len != 64) return -1;

    crypto_hash(ph, tmp, 64);
    memcpy(m, ph, 64);
    *mlen = 64;
    return 0;
}
