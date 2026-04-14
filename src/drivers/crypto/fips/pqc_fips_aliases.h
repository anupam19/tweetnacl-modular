/*
 * FIPS 203/204/205 PQC Algorithm Aliases
 * Maps legacy Kyber/Dilithium/SPHINCS+ names to FIPS standard names
 */

#ifndef PQC_FIPS_ALIASES_H
#define PQC_FIPS_ALIASES_H

#ifdef __cplusplus
extern "C" {
#endif

/* FIPS 203: ML-KEM (formerly Kyber) */
#define ML_KEM_512 PQC_KYBER512
#define ML_KEM_768 PQC_KYBER768
#define ML_KEM_1024 PQC_KYBER1024

/* FIPS 204: ML-DSA (formerly Dilithium) */
#define ML_DSA_44 PQC_DILITHIUM2
#define ML_DSA_65 PQC_DILITHIUM3
#define ML_DSA_87 PQC_DILITHIUM5

/* FIPS 205: SLH-DSA (formerly SPHINCS+) */
#define SLH_DSA_SHA2_128S PQC_SPHINCS_128S
#define SLH_DSA_SHA2_128F PQC_SPHINCS_128F

#ifdef __cplusplus
}
#endif

#endif /* PQC_FIPS_ALIASES_H */
