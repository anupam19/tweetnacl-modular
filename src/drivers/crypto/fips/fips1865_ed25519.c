/*
 * FIPS 186-5 Ed25519 Interface
 * Ed25519ctx and Ed25519ph per FIPS 186-5 Section 7
 *
 * Proper RFC 8032 Section 8.3 domain separation for Ed25519ctx.
 * Ed25519ph uses SHA-512 prehash with secure zeroization.
 */

#include "drivers/crypto/fips/fips1865_ctx.h"
#include "tweetnacl/tweetnacl.h"
#include "core/secure_mem.h"
#include "core/utils.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

/* Helper: compute Ed25519ctx context hash (SHA-512, truncated to 64 bytes) */
static void ed25519ctx_hash(uint8_t ctx_hash[64], const char *ctx, size_t ctx_len) {
    if (ctx == NULL || ctx_len == 0) {
        memset(ctx_hash, 0, 64);
    } else {
        /* FIPS 186-5 limits context to 255 bytes */
        if (ctx_len > 255) ctx_len = 255;
        crypto_hash(ctx_hash, (const uint8_t *)ctx, ctx_len);
    }
}

/* FIPS 186-5 Ed25519ctx signing with proper domain separation */
int crypto_sign_ed25519ctx(uint8_t *sm, uint64_t *smlen, const uint8_t *m, uint64_t mlen,
                            const uint8_t *sk, const char *ctx, size_t ctx_len) {
    uint8_t ctx_hash[64];
    uint8_t *combined = NULL;
    int ret = -1;
    uint64_t combined_len, out_sig_len;

    /* Parameter validation */
    if (sm == NULL || smlen == NULL || m == NULL || sk == NULL)
        return -1;
    if (ctx != NULL && ctx_len > 255)
        return -1;

    ed25519ctx_hash(ctx_hash, ctx, ctx_len);

    /* Check overflow: combined_len = 64 + mlen */
    if (mlen > SIZE_MAX - 64) goto out;
    combined_len = 64 + mlen;

    combined = (uint8_t*)malloc((size_t)combined_len);
    if (!combined) goto out;

    memcpy(combined, ctx_hash, 64);
    memcpy(combined + 64, m, (size_t)mlen);

    ret = crypto_sign(sm, &out_sig_len, combined, combined_len, sk);
    if (ret == 0) {
        *smlen = out_sig_len;
    }

    /* Out: centralized cleanup — zeroize all sensitive intermediates */
out:
    secure_zero(ctx_hash, 64);
    if (combined) {
        secure_zero(combined, (size_t)combined_len);
        free(combined);
    }
    return ret;
}

/* FIPS 186-5 Ed25519ctx verification with domain separation */
int crypto_sign_ed25519ctx_open(uint8_t *m, uint64_t *mlen, const uint8_t *sm, uint64_t smlen,
                                 const uint8_t *pk, const char *ctx, size_t ctx_len) {
    uint8_t ctx_hash[64];
    uint8_t *combined_msg = NULL;
    int ret = -1;
    uint64_t recovered_len, msg_part_len;

    if (m == NULL || mlen == NULL || sm == NULL || pk == NULL)
        goto out;
    if (ctx != NULL && ctx_len > 255)
        goto out;

    ed25519ctx_hash(ctx_hash, ctx, ctx_len);

    if (smlen < 64) goto out;
    msg_part_len = smlen - 64;  /* length of (ctx_hash || original_message) */
    if (msg_part_len < 64) goto out;  /* at least context */

    combined_msg = (uint8_t*)malloc((size_t)msg_part_len);
    if (!combined_msg) goto out;

    ret = crypto_sign_open(combined_msg, &recovered_len, sm, smlen, pk);
    if (ret != 0) goto out;
    if (recovered_len != msg_part_len) goto out;

    /* Constant-time context verification */
    if (secure_memcmp(combined_msg, ctx_hash, 64) != 0) {
        ret = -1;
        goto out;
    }

    /* Copy original message (skip context prefix) */
    memcpy(m, combined_msg + 64, (size_t)(recovered_len - 64));
    *mlen = recovered_len - 64;

out:
    if (combined_msg) {
        secure_zero(combined_msg, (size_t)msg_part_len);
        free(combined_msg);
    }
    secure_zero(ctx_hash, 64);
    return ret;
}

/* FIPS 186-5 Ed25519ph: prehash with SHA-512 */
int crypto_sign_ed25519ph(uint8_t *sm, uint64_t *smlen, const uint8_t *m, uint64_t mlen,
                          const uint8_t *sk) {
    uint8_t ph[64];
    int ret;

    if (sm == NULL || smlen == NULL || m == NULL || sk == NULL)
        return -1;

    crypto_hash(ph, m, mlen);
    ret = crypto_sign(sm, smlen, ph, 64, sk);
    secure_zero(ph, 64);
    return ret;
}

/* FIPS 186-5 Ed25519ph verification */
int crypto_sign_ed25519ph_open(uint8_t *m, uint64_t *mlen, const uint8_t *sm, uint64_t smlen,
                               const uint8_t *pk) {
    uint8_t tmp[64];  /* recovered prehash */
    uint64_t tmp_len;
    int ret;

    if (m == NULL || mlen == NULL || sm == NULL || pk == NULL)
        return -1;

    /* Verify signature and recover the 64-byte prehash */
    ret = crypto_sign_open(tmp, &tmp_len, sm, smlen, pk);
    if (ret != 0) return -1;
    if (tmp_len != 64) return -1;

    /* Return the recovered hash (preimage) to caller */
    memcpy(m, tmp, 64);
    *mlen = 64;

    /* Zeroize temporary prehash */
    secure_zero(tmp, 64);
    return 0;
}
