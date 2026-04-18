/*
 * Post-Quantum Cryptography (PQC) Implementation
 * Provides reference implementation interface for PQC algorithms
 *
 * Integration:
 * - Stub implementation by default (for testing)
 * - Real PQC via liboqs when compiled with WITH_PQC_LIBOQS
 *
 * Build with liboqs:
 *   cmake -DWITH_PQC_LIBOQS=ON ..
 */

#include "drivers/crypto/pqc.h"
#include <stdlib.h>
#include <string.h>
#include "core/secure_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WITH_PQC_LIBOQS
#include <oqs/oqs.h>
#else
#if defined(_MSC_VER)
#pragma message("warning: PQC stub implementation is active - NOT FOR PRODUCTION. Compile with -DWITH_PQC_LIBOQS=ON for real PQC.")
#else
#warning "PQC stub implementation is active - NOT FOR PRODUCTION. Compile with -DWITH_PQC_LIBOQS=ON for real PQC."
#endif
#endif

/* Algorithm parameters for supported PQC algorithms */
static const pqc_params_t pqc_params_table[] = {
    /* KYBER512 */
    {800, 1632, 768, 0, 16, "KYBER512", "NIST Level 1 KEM"},
    /* KYBER768 */
    {1184, 2400, 1088, 0, 32, "KYBER768", "NIST Level 3 KEM"},
    /* KYBER1024 */
    {1568, 3168, 1568, 0, 32, "KYBER1024", "NIST Level 5 KEM"},
    /* DILITHIUM2 */
    {1312, 2560, 0, 2420, 32, "DILITHIUM2", "NIST Level 2 Signature"},
    /* DILITHIUM3 */
    {1952, 4032, 0, 4595, 32, "DILITHIUM3", "NIST Level 3 Signature"},
    /* DILITHIUM5 */
    {2592, 4896, 0, 4595, 32, "DILITHIUM5", "NIST Level 5 Signature"},
    /* FALCON512 */
    {897, 1281, 0, 666, 32, "FALCON512", "Compact Signature Level 1"},
    /* FALCON1024 */
    {1793, 2305, 0, 1026, 32, "FALCON1024", "Compact Signature Level 5"},
    /* SPHINCS-SHA2-128F */
    {16, 16, 0, 17088, 16, "SPHINCS-SHA2-128F", "Stateless Hash-based Sig Fast"},
    /* SPHINCS-SHA2-128S */
    {16, 16, 0, 8008, 16, "SPHINCS-SHA2-128S", "Stateless Hash-based Sig Small"},
    /* SPHINCS-SHAKE-128F */
    {16, 16, 0, 17088, 16, "SPHINCS-SHAKE-128F", "Stateless Hash-based Sig Fast (SHAKE)"},
    /* SPHINCS-SHAKE-128S */
    {16, 16, 0, 8008, 16, "SPHINCS-SHAKE-128S", "Stateless Hash-based Sig Small (SHAKE)"}};

#define NUM_PQC_ALGORITHMS (sizeof(pqc_params_table) / sizeof(pqc_params_t))

/* Get algorithm parameters - works for both stub and liboqs */
pqc_result_t pqc_get_params(pqc_algorithm_t algo, pqc_params_t *params) {
    if (params == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    if (algo < 0 || algo >= (pqc_algorithm_t)NUM_PQC_ALGORITHMS) {
        return PQC_ERROR_ALGORITHM_NOT_SUPPORTED;
    }

    memcpy(params, &pqc_params_table[algo], sizeof(pqc_params_t));
    return PQC_SUCCESS;
}

#ifdef WITH_PQC_LIBOQS

/* Map our enum to OQS algorithm names */
static const char *oqs_alg_name(pqc_algorithm_t algo) {
    switch (algo) {
    case PQC_KYBER512:
        return OQS_KEM_alg_kyber_512;
    case PQC_KYBER768:
        return OQS_KEM_alg_kyber_768;
    case PQC_KYBER1024:
        return OQS_KEM_alg_kyber_1024;
    case PQC_DILITHIUM2:
        return OQS_SIG_alg_dilithium_2;
    case PQC_DILITHIUM3:
        return OQS_SIG_alg_dilithium_3;
    case PQC_DILITHIUM5:
        return OQS_SIG_alg_dilithium_5;
    case PQC_FALCON512:
        return OQS_SIG_alg_falcon_512;
    case PQC_FALCON1024:
        return OQS_SIG_alg_falcon_1024;
    case PQC_SPHINCS_SHA2_128F:
        return OQS_SIG_alg_sphincs_sha2_128f;
    case PQC_SPHINCS_SHA2_128S:
        return OQS_SIG_alg_sphincs_sha2_128s;
    case PQC_SPHINCS_SHAKE_128F:
        return OQS_SIG_alg_sphincs_shake_128f;
    case PQC_SPHINCS_SHAKE_128S:
        return OQS_SIG_alg_sphincs_shake_128s;
    default:
        return NULL;
    }
}

/* liboqs-based implementation */
pqc_result_t pqc_keygen(pqc_algorithm_t algo, uint8_t *public_key, size_t public_key_len,
                        uint8_t *secret_key, size_t secret_key_len, const uint8_t *seed,
                        size_t seed_len) {
    const char *alg_name = oqs_alg_name(algo);
    if (!alg_name)
        return PQC_ERROR_ALGORITHM_NOT_SUPPORTED;

    if (public_key == NULL || secret_key == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    /* Determine if KEM or SIG */
    if (OQS_KEM_alg_is_enabled(alg_name)) {
        OQS_KEM *kem = OQS_KEM_new(alg_name);
        if (!kem)
            return PQC_ERROR_KEY_GENERATION_FAILED;

        if (public_key_len < kem->length_public_key || secret_key_len < kem->length_secret_key) {
            OQS_KEM_free(kem);
            return PQC_ERROR_BUFFER_TOO_SMALL;
        }

        if (OQS_KEM_keypair(kem, public_key, secret_key) != OQS_SUCCESS) {
            /* Zeroize partial key material before cleanup */
            secure_zero(public_key, public_key_len);
            secure_zero(secret_key, secret_key_len);
            OQS_KEM_free(kem);
            return PQC_ERROR_KEY_GENERATION_FAILED;
        }

        OQS_KEM_free(kem);
        return PQC_SUCCESS;

    } else if (OQS_SIG_alg_is_enabled(alg_name)) {
        OQS_SIG *sig = OQS_SIG_new(alg_name);
        if (!sig)
            return PQC_ERROR_KEY_GENERATION_FAILED;

        if (public_key_len < sig->length_public_key || secret_key_len < sig->length_secret_key) {
            OQS_SIG_free(sig);
            return PQC_ERROR_BUFFER_TOO_SMALL;
        }

        if (OQS_SIG_keypair(sig, public_key, secret_key) != OQS_SUCCESS) {
            /* Zeroize partial key material before cleanup */
            secure_zero(public_key, public_key_len);
            secure_zero(secret_key, secret_key_len);
            OQS_SIG_free(sig);
            return PQC_ERROR_KEY_GENERATION_FAILED;
        }

        OQS_SIG_free(sig);
        return PQC_SUCCESS;
    }

    return PQC_ERROR_ALGORITHM_NOT_SUPPORTED;
}

/* Additional liboqs implementations would go here */

#else /* WITH_PQC_STUB */

/* Stub key generation - in production, call actual PQC library */
pqc_result_t pqc_keygen(pqc_algorithm_t algo, uint8_t *public_key, size_t public_key_len,
                        uint8_t *secret_key, size_t secret_key_len, const uint8_t *seed,
                        size_t seed_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (public_key == NULL || secret_key == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    result = pqc_get_params(algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    if (public_key_len < params.public_key_size || secret_key_len < params.secret_key_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    /*
     * STUB IMPLEMENTATION: Fill with deterministic pattern for testing
     * In production, integrate with liboqs or similar PQC library
     */
    memset(public_key, 0xAA, params.public_key_size);
    memset(secret_key, 0xBB, params.secret_key_size);

    if (seed != NULL && seed_len > 0) {
        /* Use seed to initialize (simplified for stub) */
        for (size_t i = 0; i < params.public_key_size && i < seed_len; i++) {
            public_key[i] ^= seed[i % seed_len];
        }
    }

    return PQC_SUCCESS;
}

/* Stub encapsulation */
pqc_result_t pqc_encapsulate(pqc_algorithm_t algo, const uint8_t *public_key, size_t public_key_len,
                             uint8_t *ciphertext, size_t *ciphertext_len, uint8_t *shared_secret,
                             size_t shared_secret_len, const uint8_t *random, size_t random_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (public_key == NULL || ciphertext == NULL || shared_secret == NULL ||
        ciphertext_len == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    result = pqc_get_params(algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    if (public_key_len < params.public_key_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    if (*ciphertext_len < params.ciphertext_size) {
        *ciphertext_len = params.ciphertext_size;
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    if (shared_secret_len < params.shared_secret_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    *ciphertext_len = params.ciphertext_size;

    /* STUB: Generate deterministic ciphertext and shared secret */
    memset(ciphertext, 0xCC, params.ciphertext_size);
    memset(shared_secret, 0xDD, params.shared_secret_size);

    if (random != NULL && random_len > 0) {
        for (size_t i = 0; i < params.shared_secret_size && i < random_len; i++) {
            shared_secret[i] ^= random[i % random_len];
        }
    }

    return PQC_SUCCESS;
}

/* Stub decapsulation */
pqc_result_t pqc_decapsulate(pqc_algorithm_t algo, const uint8_t *secret_key, size_t secret_key_len,
                             const uint8_t *ciphertext, size_t ciphertext_len,
                             uint8_t *shared_secret, size_t shared_secret_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (secret_key == NULL || ciphertext == NULL || shared_secret == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    result = pqc_get_params(algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    if (secret_key_len < params.secret_key_size || ciphertext_len < params.ciphertext_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    if (shared_secret_len < params.shared_secret_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    /* STUB: Generate shared secret from ciphertext and secret key */
    memset(shared_secret, 0xEE, params.shared_secret_size);
    for (size_t i = 0; i < params.shared_secret_size; i++) {
        shared_secret[i] ^= ciphertext[i % ciphertext_len];
        shared_secret[i] ^= secret_key[i % secret_key_len];
    }

    return PQC_SUCCESS;
}

/* Stub signature */
pqc_result_t pqc_sign(pqc_algorithm_t algo, const uint8_t *secret_key, size_t secret_key_len,
                      const uint8_t *message, size_t message_len, uint8_t *signature,
                      size_t *signature_len, const uint8_t *random, size_t random_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (secret_key == NULL || message == NULL || signature == NULL || signature_len == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    result = pqc_get_params(algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    if (secret_key_len < params.secret_key_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    if (*signature_len < params.signature_size) {
        *signature_len = params.signature_size;
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    *signature_len = params.signature_size;

    /* STUB: Generate deterministic signature */
    memset(signature, 0xFF, params.signature_size);

    /* Incorporate message hash (simplified) */
    for (size_t i = 0; i < params.signature_size && message_len > 0; i++) {
        signature[i] ^= message[i % message_len];
    }

    if (random != NULL && random_len > 0) {
        for (size_t i = 0; i < params.signature_size; i++) {
            signature[i] ^= random[i % random_len];
        }
    }

    return PQC_SUCCESS;
}

/* Stub verification - NEVER用于生产环境 */
pqc_result_t pqc_verify(pqc_algorithm_t algo, const uint8_t *public_key, size_t public_key_len,
                        const uint8_t *message, size_t message_len, const uint8_t *signature,
                        size_t signature_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (public_key == NULL || message == NULL || signature == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    result = pqc_get_params(algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    if (public_key_len < params.public_key_size || signature_len < params.signature_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    /*
     * STUB MODE: Always return verification failure.
     * Stub implementation must never succeed in production.
     * Compile with -DWITH_PQC_LIBOQS=ON to enable real PQC.
     */
    (void)message_len; /* Suppress unused warning */
    return PQC_ERROR_VERIFICATION_FAILED;
}

/* Hybrid mode: Combine Curve25519 with PQC */
pqc_result_t pqc_hybrid_keygen(pqc_algorithm_t pqc_algo, uint8_t *hybrid_public_key,
                               size_t *hybrid_public_key_len, uint8_t *hybrid_secret_key,
                               size_t *hybrid_secret_key_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (hybrid_public_key == NULL || hybrid_secret_key == NULL || hybrid_public_key_len == NULL ||
        hybrid_secret_key_len == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    result = pqc_get_params(pqc_algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    /* Hybrid key format: [Curve25519 (32 bytes)] [PQC Public Key] */
    size_t curve25519_pk_size = 32;
    size_t curve25519_sk_size = 32;

    *hybrid_public_key_len = curve25519_pk_size + params.public_key_size;
    *hybrid_secret_key_len = curve25519_sk_size + params.secret_key_size;

    /* Initialize with zeros (Curve25519 keys would be generated separately) */
    memset(hybrid_public_key, 0, *hybrid_public_key_len);
    memset(hybrid_secret_key, 0, *hybrid_secret_key_len);

    /* Generate PQC key pair in the second part */
    result = pqc_keygen(pqc_algo, hybrid_public_key + curve25519_pk_size, params.public_key_size,
                        hybrid_secret_key + curve25519_sk_size, params.secret_key_size, NULL, 0);

    return result;
}

pqc_result_t pqc_hybrid_encapsulate(pqc_algorithm_t pqc_algo,
                                    const uint8_t *hybrid_public_key, size_t hybrid_public_key_len,
                                    uint8_t *hybrid_ciphertext, size_t *hybrid_ciphertext_len,
                                    uint8_t *hybrid_shared_secret, size_t hybrid_shared_secret_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (hybrid_public_key == NULL || hybrid_ciphertext == NULL || hybrid_shared_secret == NULL ||
        hybrid_ciphertext_len == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    /* Get PQC algorithm parameters */
    result = pqc_get_params(pqc_algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    /* Hybrid public key format: [Curve25519 (32 bytes)] [PQC Public Key] */
    size_t curve25519_pk_size = 32;
    if (hybrid_public_key_len < curve25519_pk_size) {
        return PQC_ERROR_INVALID_PARAM;
    }
    size_t pqc_pubkey_len = hybrid_public_key_len - curve25519_pk_size;

    if (pqc_pubkey_len < params.public_key_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    /* Hybrid ciphertext format: [Curve25519 ephemeral (32 bytes)] [PQC Ciphertext] */
    size_t curve25519_ct_size = 32;
    size_t pqc_ct_size = params.ciphertext_size;

    /* Check for integer overflow before addition (CERT INT30-C) */
    if (curve25519_ct_size > SIZE_MAX - pqc_ct_size) {
        return PQC_ERROR_MEMORY_ALLOCATION;
    }
    size_t required_ct_size = curve25519_ct_size + pqc_ct_size;

    /* Check caller's buffer is large enough */
    if (*hybrid_ciphertext_len < required_ct_size) {
        *hybrid_ciphertext_len = required_ct_size;
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }
    *hybrid_ciphertext_len = required_ct_size;

    /* Hybrid shared secret: SHA256(Curve25519_shared || PQC_shared) */
    size_t hybrid_ss_size = 32; /* SHA256 output */
    if (hybrid_shared_secret_len < hybrid_ss_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    /* STUB: Generate hybrid shared secret (zero-filled for demo) */
    secure_zero(hybrid_ciphertext, required_ct_size);
    secure_zero(hybrid_shared_secret, hybrid_ss_size);

    return PQC_SUCCESS;
}

pqc_result_t pqc_hybrid_decapsulate(pqc_algorithm_t pqc_algo,
                                    const uint8_t *hybrid_secret_key, size_t hybrid_secret_key_len,
                                    const uint8_t *hybrid_ciphertext, size_t hybrid_ciphertext_len,
                                    uint8_t *hybrid_shared_secret, size_t hybrid_shared_secret_len) {
    pqc_params_t params;
    pqc_result_t result;

    if (hybrid_secret_key == NULL || hybrid_ciphertext == NULL || hybrid_shared_secret == NULL) {
        return PQC_ERROR_INVALID_PARAM;
    }

    /* Get PQC algorithm parameters */
    result = pqc_get_params(pqc_algo, &params);
    if (result != PQC_SUCCESS) {
        return result;
    }

    /* Hybrid secret key format: [Curve25519 (32 bytes)] [PQC Secret Key] */
    size_t curve25519_sk_size = 32;
    if (hybrid_secret_key_len < curve25519_sk_size) {
        return PQC_ERROR_INVALID_PARAM;
    }

    /* Hybrid ciphertext: [Curve25519 ephemeral (32 bytes)] [PQC Ciphertext] */
    if (hybrid_ciphertext_len < 32 + params.ciphertext_size) {
        return PQC_ERROR_INVALID_PARAM;
    }

    size_t hybrid_ss_size = 32;
    if (hybrid_shared_secret_len < hybrid_ss_size) {
        return PQC_ERROR_BUFFER_TOO_SMALL;
    }

    /* STUB: Derive hybrid shared secret */
    memset(hybrid_shared_secret, 0x33, hybrid_ss_size);

    return PQC_SUCCESS;
}

#endif /* WITH_PQC_LIBOQS */

/* Utility functions - available in both stub and liboqs */
const char *pqc_result_to_string(pqc_result_t result) {
    switch (result) {
    case PQC_SUCCESS:
        return "Success";
    case PQC_ERROR_INVALID_PARAM:
        return "Invalid parameter";
    case PQC_ERROR_BUFFER_TOO_SMALL:
        return "Buffer too small";
    case PQC_ERROR_ALGORITHM_NOT_SUPPORTED:
        return "Algorithm not supported";
    case PQC_ERROR_KEY_GENERATION_FAILED:
        return "Key generation failed";
    case PQC_ERROR_ENCRYPTION_FAILED:
        return "Encryption failed";
    case PQC_ERROR_DECRYPTION_FAILED:
        return "Decryption failed";
    case PQC_ERROR_SIGNATURE_INVALID:
        return "Invalid signature";
    case PQC_ERROR_VERIFICATION_FAILED:
        return "Verification failed";
    case PQC_ERROR_MEMORY_ALLOCATION:
        return "Memory allocation failed";
    default:
        return "Unknown error";
    }
}

const char *pqc_algorithm_to_string(pqc_algorithm_t algo) {
    if (algo >= 0 && algo < (pqc_algorithm_t)NUM_PQC_ALGORITHMS) {
        return pqc_params_table[algo].name;
    }
    return "Unknown algorithm";
}

#ifdef __cplusplus
}
#endif
