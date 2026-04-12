/*
 * ARM32 (ARMv7) CPU Feature Detection
 * Detects: NEON, VFPv3, AES, SHA1, SHA2
 */

#ifndef ARCH_ARM32_CPU_FEATURES_H
#define ARCH_ARM32_CPU_FEATURES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CPU feature flags */
#define CPU_ARM32_NEON    (1u << 0)  /* NEON SIMD */
#define CPU_ARM32_VFPv3   (1u << 1)  /* VFPv3 floating point */
#define CPU_ARM32_AES     (1u << 2)  /* AES crypto extension */
#define CPU_ARM32_SHA1    (1u << 3)  /* SHA-1 crypto extension */
#define CPU_ARM32_SHA2    (1u << 4)  /* SHA-2 crypto extension */
#define CPU_ARM32_PMULL   (1u << 5)  /* Polynomial multiply */

/* Detect available CPU features via HWCAP */
uint32_t arm32_detect_features(void);

/* Check if a specific feature is available */
static inline int arm32_has_feature(uint32_t feature) {
    static uint32_t cached = 0;
    if (!cached) cached = arm32_detect_features();
    return (cached & feature) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ARCH_ARM32_CPU_FEATURES_H */
