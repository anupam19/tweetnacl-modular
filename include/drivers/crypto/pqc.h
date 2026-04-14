/*
 * Post-Quantum Cryptography (PQC) Interface Header
 * Provides abstraction for PQC algorithms
 */

#ifndef PQC_H
#define PQC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PQC Algorithm Identifiers */
typedef enum {
    PQC_KYBER512 = 0,
    PQC_KYBER768 = 1,
    PQC_KYBER1024 = 2,
    PQC_DILITHIUM2 = 3,
    PQC_DILITHIUM3 = 4,
    PQC_DILITHIUM5 = 5,
    PQC_FALCON512 = 6,
    PQC_FALCON1024 = 7,
    PQC_SPHINCS_SHA2_128F = 8,
    PQC_SPHINCS_SHA2_128S = 9,
    PQC_SPHINCS_SHAKE_128F = 10,
    PQC_SPHINCS_SHAKE_128S = 11
} pqc_algorithm_t;

/* PQC Key Types */
typedef enum { PQC_KEY_PUBLIC = 0, PQC_KEY_SECRET = 1, PQC_KEY_SHARED = 2 } pqc_key_type_t;

/* PQC Operation Results */
typedef enum {
    PQC_SUCCESS = 0,
    PQC_ERROR_INVALID_PARAM = -1,
    PQC_ERROR_BUFFER_TOO_SMALL = -2,
    PQC_ERROR_ALGORITHM_NOT_SUPPORTED = -3,
    PQC_ERROR_KEY_GENERATION_FAILED = -4,
    PQC_ERROR_ENCRYPTION_FAILED = -5,
    PQC_ERROR_DECRYPTION_FAILED = -6,
    PQC_ERROR_SIGNATURE_INVALID = -7,
    PQC_ERROR_VERIFICATION_FAILED = -8,
    PQC_ERROR_MEMORY_ALLOCATION = -9
} pqc_result_t;

/* PQC Algorithm Parameters */
typedef struct {
    size_t public_key_size;
    size_t secret_key_size;
    size_t ciphertext_size;
    size_t signature_size;
    size_t shared_secret_size;
    const char *name;
    const char *description;
} pqc_params_t;

/* Get algorithm parameters */
pqc_result_t pqc_get_params(pqc_algorithm_t algo, pqc_params_t *params);

/* Key Generation */
pqc_result_t pqc_keygen(pqc_algorithm_t algo, uint8_t *public_key, size_t public_key_len,
                        uint8_t *secret_key, size_t secret_key_len, const uint8_t *seed,
                        size_t seed_len);

/* Encapsulation (KEM) */
pqc_result_t pqc_encapsulate(pqc_algorithm_t algo, const uint8_t *public_key, size_t public_key_len,
                             uint8_t *ciphertext, size_t *ciphertext_len, uint8_t *shared_secret,
                             size_t shared_secret_len, const uint8_t *random, size_t random_len);

pqc_result_t pqc_decapsulate(pqc_algorithm_t algo, const uint8_t *secret_key, size_t secret_key_len,
                             const uint8_t *ciphertext, size_t ciphertext_len,
                             uint8_t *shared_secret, size_t shared_secret_len);

/* Digital Signatures */
pqc_result_t pqc_sign(pqc_algorithm_t algo, const uint8_t *secret_key, size_t secret_key_len,
                      const uint8_t *message, size_t message_len, uint8_t *signature,
                      size_t *signature_len, const uint8_t *random, size_t random_len);

pqc_result_t pqc_verify(pqc_algorithm_t algo, const uint8_t *public_key, size_t public_key_len,
                        const uint8_t *message, size_t message_len, const uint8_t *signature,
                        size_t signature_len);

/* Hybrid Mode: Combine classical (Curve25519) with PQC */
pqc_result_t pqc_hybrid_keygen(pqc_algorithm_t pqc_algo, uint8_t *hybrid_public_key,
                               size_t *hybrid_public_key_len, uint8_t *hybrid_secret_key,
                               size_t *hybrid_secret_key_len);

pqc_result_t pqc_hybrid_encapsulate(const uint8_t *hybrid_public_key, size_t hybrid_public_key_len,
                                    uint8_t *hybrid_ciphertext, size_t *hybrid_ciphertext_len,
                                    uint8_t *hybrid_shared_secret, size_t hybrid_shared_secret_len);

pqc_result_t pqc_hybrid_decapsulate(const uint8_t *hybrid_secret_key, size_t hybrid_secret_key_len,
                                    const uint8_t *hybrid_ciphertext, size_t hybrid_ciphertext_len,
                                    uint8_t *hybrid_shared_secret, size_t hybrid_shared_secret_len);

/* Utility Functions */
const char *pqc_result_to_string(pqc_result_t result);
const char *pqc_algorithm_to_string(pqc_algorithm_t algo);

#ifdef __cplusplus
}
#endif

#endif /* PQC_H */
