/*
 * RISC-V 64-bit CPU Feature Detection
 * Detects: Vector extension (V), Scalar Crypto extensions (Zk*)
 */

#ifndef ARCH_RISCV64_CPU_FEATURES_H
#define ARCH_RISCV64_CPU_FEATURES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CPU feature flags */
#define CPU_RISCV_V (1u << 0)    /* Vector extension */
#define CPU_RISCV_ZKND (1u << 1) /* AES decryption */
#define CPU_RISCV_ZKNE (1u << 2) /* AES encryption */
#define CPU_RISCV_ZKNH (1u << 3) /* SHA-256/SHA-512 */
#define CPU_RISCV_ZKR (1u << 4)  /* Entropy source */
#define CPU_RISCV_ZKS (1u << 5)  /* SM4/SM3 */

/* Detect available CPU features via misa and mvendorid */
uint32_t riscv64_detect_features(void);

/* Check if a specific feature is available */
static inline int riscv64_has_feature(uint32_t feature) {
    static uint32_t cached = 0;
    if (!cached)
        cached = riscv64_detect_features();
    return (cached & feature) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ARCH_RISCV64_CPU_FEATURES_H */
